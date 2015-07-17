# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import posixpath
import re

from telemetry.timeline import event as timeline_event


class MmapCategory(object):
  _DEFAULT_CATEGORY = None

  def __init__(self, name, file_pattern, children=None):
    """A (sub)category for classifying memory maps.

    Args:
      name: A string to identify the category.
      file_pattern: A regex pattern, the category will aggregate memory usage
          for all mapped files matching this pattern.
      children: A list of MmapCategory objects, used to sub-categorize memory
          usage.
    """
    self.name = name
    self._file_pattern = re.compile(file_pattern) if file_pattern else None
    self._children = list(children) if children else None

  @classmethod
  def DefaultCategory(cls):
    """An implicit 'Others' match-all category with no children."""
    if cls._DEFAULT_CATEGORY is None:
      cls._DEFAULT_CATEGORY = cls('Others', None)
    return cls._DEFAULT_CATEGORY

  def Match(self, mapped_file):
    """Test whether a mapped file matches this category."""
    return (self._file_pattern is None
            or bool(self._file_pattern.search(mapped_file)))

  def GetMatchingChild(self, mapped_file):
    """Get the first matching sub-category for a given mapped file.

    Returns None if the category has no children, or the DefaultCategory if
    it does have children but none of them match.
    """
    if not self._children:
      return None
    for child in self._children:
      if child.Match(mapped_file):
        return child
    return type(self).DefaultCategory()


ROOT_CATEGORY = MmapCategory('/', None, [
  MmapCategory('Android', r'^\/dev\/ashmem(?!\/libc malloc)', [
    MmapCategory('Java runtime', r'^\/dev\/ashmem\/dalvik-', [
      MmapCategory('Spaces', r'\/dalvik-(alloc|main|large'
                             r' object|non moving|zygote) space', [
        MmapCategory('Normal', r'\/dalvik-(alloc|main)'),
        MmapCategory('Large', r'\/dalvik-large object'),
        MmapCategory('Zygote', r'\/dalvik-zygote'),
        MmapCategory('Non-moving', r'\/dalvik-non moving')
      ]),
      MmapCategory('Linear Alloc', r'\/dalvik-LinearAlloc'),
      MmapCategory('Indirect Reference Table', r'\/dalvik-indirect.ref'),
      MmapCategory('Cache', r'\/dalvik-jit-code-cache'),
      MmapCategory('Accounting', None)
    ]),
    MmapCategory('Cursor', r'\/CursorWindow'),
    MmapCategory('Ashmem', None)
  ]),
  MmapCategory('Native heap',
               r'^((\[heap\])|(\[anon:)|(\/dev\/ashmem\/libc malloc)|$)'),
  MmapCategory('Stack', r'^\[stack'),
  MmapCategory('Files',
               r'\.((((so)|(jar)|(apk)|(ttf)|(odex)|(oat)|(arg))$)|(dex))', [
    MmapCategory('so', r'\.so$'),
    MmapCategory('jar', r'\.jar$'),
    MmapCategory('apk', r'\.apk$'),
    MmapCategory('ttf', r'\.ttf$'),
    MmapCategory('dex', r'\.((dex)|(odex$))'),
    MmapCategory('oat', r'\.oat$'),
    MmapCategory('art', r'\.art$'),
  ]),
  MmapCategory('Devices', r'(^\/dev\/)|(anon_inode:dmabuf)', [
    MmapCategory('GPU', r'\/((nv)|(mali)|(kgsl))'),
    MmapCategory('DMA', r'anon_inode:dmabuf'),
  ]),
  MmapCategory('Discounted tracing overhead',
               r'\[discounted tracing overhead\]')
])


# Map long descriptive attribute names, as understood by MemoryBucket.GetValue,
# to the short keys used by events in raw json traces.
BUCKET_ATTRS = {
  'proportional_resident': 'pss',
  'private_dirty_resident': 'pd',
  'private_clean_resident': 'pc',
  'shared_dirty_resident': 'sd',
  'shared_clean_resident': 'sc',
  'swapped': 'sw'}


# Map of {memory_key: (category_path, discount_tracing), ...}.
# When discount_tracing is True, we have to discount the resident_size of the
# tracing allocator to get the correct value for that key.
MMAPS_METRICS = {
  'mmaps_overall_pss': ('/.proportional_resident', True),
  'mmaps_private_dirty' : ('/.private_dirty_resident', True),
  'mmaps_java_heap': ('/Android/Java runtime/Spaces.proportional_resident',
                      False),
  'mmaps_ashmem': ('/Android/Ashmem.proportional_resident', False),
  'mmaps_native_heap': ('/Native heap.proportional_resident', True)}


class MemoryBucket(object):
  """Simple object to hold and aggregate memory values."""
  def __init__(self):
    self._bucket = dict.fromkeys(BUCKET_ATTRS.iterkeys(), 0)

  def __repr__(self):
    values = ', '.join('%s=%d' % (src_key, self._bucket[dst_key])
                       for dst_key, src_key
                       in sorted(BUCKET_ATTRS.iteritems()))
    return '%s[%s]' % (type(self).__name__, values)

  def AddRegion(self, byte_stats):
    for dst_key, src_key in BUCKET_ATTRS.iteritems():
      self._bucket[dst_key] += int(byte_stats.get(src_key, '0'), 16)

  def GetValue(self, name):
    return self._bucket[name]


class ProcessMemoryDumpEvent(timeline_event.TimelineEvent):
  """A memory dump event belonging to a single timeline.Process object.

  It's a subclass of telemetry's TimelineEvent so it can be included in
  the stream of events contained in timeline.model objects, and have its
  timing correlated with that of other events in the model.

  Properties:
    dump_id: A string to identify events belonging to the same global dump.
    process: The timeline.Process object that owns this memory dump event.
    has_mmaps: True if the memory dump has mmaps information. If False then
        GetStatsSummary will report all zeros.
  """
  def __init__(self, process, event):
    assert event['ph'] == 'v' and process.pid == event['pid']

    super(ProcessMemoryDumpEvent, self).__init__(
        'memory', 'memory_dump', event['ts'] / 1000.0, 0.0)

    self.process = process
    self.dump_id = event['id']

    try:
      allocators_dict = event['args']['dumps']['allocators']
    except KeyError:
      allocators_dict = {}
    # populate keys that should always be present
    self._allocators = {}
    for allocator_name, size_values in allocators_dict.iteritems():
      name_parts = allocator_name.split('/')
      # we want to skip allocated_objects, since they are already counted by
      # outer allocator names; but malloc is special, because the size of outer
      # allocators is only inherited from its allocated_objects.
      if name_parts[-1] == 'allocated_objects' and name_parts[0] != 'malloc':
        continue
      allocator_name = name_parts[0]
      allocator = self._allocators.setdefault(allocator_name, {})
      for size_key, size_value in size_values['attrs'].iteritems():
        allocator[size_key] = (allocator.get(size_key, 0)
                               + int(size_value['value'], 16))
    # we need to discount tracing from malloc size.
    try:
      self._allocators['malloc']['size'] -= self._allocators['tracing']['size']
    except KeyError:
      pass # it's ok if any of those keys are not present

    self._buckets = {}
    try:
      vm_regions = event['args']['dumps']['process_mmaps']['vm_regions']
    except KeyError:
      vm_regions = []
    self.has_mmaps = bool(vm_regions)
    for vm_region in vm_regions:
      self._AddRegion(vm_region)

  @property
  def process_name(self):
    return self.process.name

  def _AddRegion(self, vm_region):
    path = ''
    category = ROOT_CATEGORY
    while category:
      path = posixpath.join(path, category.name)
      self.GetMemoryBucket(path).AddRegion(vm_region['bs'])
      mapped_file = vm_region['mf']
      category = category.GetMatchingChild(mapped_file)

  def __repr__(self):
    values = ['pid=%d' % self.pid]
    for key, value in sorted(self.GetStatsSummary().iteritems()):
      values.append('%s=%d' % (key, value))
    values = ', '.join(values)
    return '%s[%s]' % (type(self).__name__, values)

  def GetMemoryBucket(self, path):
    """Return the MemoryBucket associated with a category path.

    An empty bucket will be created if the path does not already exist.

    path: A string with path in the classification tree, e.g.
        '/Android/Java runtime/Cache'. Note: no trailing slash, except for
        the root path '/'.
    """
    if not path in self._buckets:
      self._buckets[path] = MemoryBucket()
    return self._buckets[path]

  def GetMemoryValue(self, category_path, discount_tracing=False):
    """Return a specific value from within a MemoryBucket.

    category_path: A string composed of a path in the classification tree,
        followed by a '.', followed by a specific bucket value, e.g.
        '/Android/Java runtime/Cache.private_dirty_resident'.
    discount_tracing: A boolean indicating whether the returned value should
        be discounted by the resident size of the tracing allocator.
    """
    path, name = category_path.rsplit('.', 1)
    value = self.GetMemoryBucket(path).GetValue(name)
    if discount_tracing and 'tracing' in self._allocators:
      value -= self._allocators['tracing'].get('resident_size', 0)
    return value

  def GetMemoryUsage(self):
    """Get a dictionary with the memory usage of this process."""
    usage = {'allocator_%s' % name: allocator.get('size', 0)
             for name, allocator in self._allocators.iteritems()}
    if self.has_mmaps:
      usage.update((key, self.GetMemoryValue(*value))
                   for key, value in MMAPS_METRICS.iteritems())
    return usage


class GlobalMemoryDump(object):
  """Object to aggregate individual process dumps with the same dump id.

  Args:
    process_dumps: A sequence of ProcessMemoryDumpEvent objects, all sharing
        the same global dump id.

  Attributes:
    dump_id: A string identifying this dump.
    has_mmaps: True if the memory dump has mmaps information. If False then
        GetStatsSummary will report all zeros.
  """
  def __init__(self, process_dumps):
    assert process_dumps
    # Keep dumps sorted in chronological order.
    self._process_dumps = sorted(process_dumps, key=lambda dump: dump.start)

    # All process dump events should have the same dump id.
    dump_ids = set(dump.dump_id for dump in self._process_dumps)
    assert len(dump_ids) == 1
    self.dump_id = dump_ids.pop()

    # Either all processes have mmaps or none of them do.
    have_mmaps = set(dump.has_mmaps for dump in self._process_dumps)
    assert len(have_mmaps) == 1
    self.has_mmaps = have_mmaps.pop()

  @property
  def start(self):
    return self._process_dumps[0].start

  @property
  def end(self):
    return self._process_dumps[-1].start

  @property
  def duration(self):
    return self.end - self.start

  def IterProcessMemoryDumps(self):
    return iter(self._process_dumps)

  def __repr__(self):
    values = ['id=%s' % self.dump_id]
    for key, value in sorted(self.GetStatsSummary().iteritems()):
      values.append('%s=%d' % (key, value))
    values = ', '.join(values)
    return '%s[%s]' % (type(self).__name__, values)

  def GetMemoryUsage(self):
    """Get the aggregated memory usage over all processes in this dump."""
    result = {}
    for dump in self._process_dumps:
      for key, value in dump.GetMemoryUsage().iteritems():
        result[key] = result.get(key, 0) + value
    return result
