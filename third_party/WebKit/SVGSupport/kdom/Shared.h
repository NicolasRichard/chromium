/*
    Copyright (C) 2004 Nikolas Zimmermann <wildfox@kde.org>
				  2004 Rob Buis <buis@kde.org>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KDOM_Shared_H
#define KDOM_Shared_H

namespace KDOM
{
	// The class expects 'this' to be an ecmafied class which needs
	// to call ScriptInterpreter::forgetDOMObject on destruction...
	class Shared
	{
	public:
		Shared(bool baseClass = false);
		virtual ~Shared();

		void ref();
		virtual void deref();

		int refCount() const;

	protected:
		int m_ref;
		bool m_baseClass : 1;
	};

	template<class T>
	inline void KDOM_SAFE_SET(T *&a, T *b)
	{
		if(a != b)
		{
			if(a) a->deref();
			a = b;
			if(a) a->ref();
		}
	}

#define KDOM_IMPL_DTOR_ASSIGN_OP(T) \
T::~T() { if(d) d->deref(); } \
T &T::operator=(const T &other) { \
	KDOM_SAFE_SET(d, other.d); \
	return *this; \
} \
bool T::operator==(const T &other) const { \
	return d == other.d; \
} \
bool T::operator!=(const T &other) const { \
	return !operator==(other); \
} \

/**
 * Add a null object singleton to the class, of the class' type.
 * This makes for efficient storage and can be used in comparisons
 * like this: someNode.firstChild() != Node::null; .
 *
 * TODO: is there demand for isNull() convenience method? If so, add
 * here.
 */
#define KDOM_INTERNAL(ClassName) static ClassName null; typedef ClassName##Impl Private;

/**
 * Base classes have a handle() convenience method that gives back
 * the internal impl pointer, in addition to the null object singleton.
 */
#define KDOM_INTERNAL_BASE(ClassName) KDOM_INTERNAL(ClassName) ClassName##Impl *handle() const { return d; }

};

#endif

// vim:ts=4:noet
