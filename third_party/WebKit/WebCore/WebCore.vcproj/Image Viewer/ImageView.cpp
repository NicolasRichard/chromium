// ImageView.cpp : implementation of the ImageView class
//

#include "stdafx.h"
#include "Image Viewer.h"
#include "ImageDocument.h"
#include "ImageView.h"
#include <cairo.h>
#include "Image.h"
#include "FloatRect.h"
#include "IntSize.h"

using WebCore::Image;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ImageView

IMPLEMENT_DYNCREATE(ImageView, CView)

BEGIN_MESSAGE_MAP(ImageView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
        ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// ImageView construction/destruction

ImageView::ImageView()
{
}

ImageView::~ImageView()
{
}

BOOL ImageView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// ImageView drawing

BOOL ImageView::OnEraseBkgnd(CDC* pDC)
{
    // Don't ever let windows erase the background.
    ImageDocument* pDoc = GetDocument();
    if (!pDoc || !pDoc->image())
        return CView::OnEraseBkgnd(pDC);
    return FALSE;
}

void ImageView::OnDraw(CDC* pDC)
{
    ImageDocument* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    Image* image = pDoc->image();
    if (!image)
        return;

    // Make a win32 cairo surface for painting.
    HDC dc = pDC->m_hDC;
    
    // Center the image.
    RECT rect;
    GetClientRect(&rect);

    cairo_surface_t* finalSurface = cairo_win32_surface_create(dc);
    cairo_surface_t* surface = cairo_surface_create_similar(finalSurface,
                                                            CAIRO_CONTENT_COLOR_ALPHA,
                                                            rect.right, rect.bottom);

    // Fill with white.
    cairo_t* context = cairo_create(surface);
    cairo_rectangle(context, 0, 0, rect.right, rect.bottom);
    cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
    cairo_fill(context);

    // Comment in to test scaling (doubles the size).
    float width = image->size().width(); // * 2;
    float height = image->size().height(); // * 2;

    float left = (rect.right - width) / 2;
    float top = (rect.bottom - height) / 2;
    if (left < 0) left = 0;
    if (top < 0) top = 0;

    FloatRect dstRect(FloatPoint(left, top), FloatSize(width, height));
    FloatRect imageRect(FloatPoint(), image->size());
    image->drawInRect(dstRect, imageRect, Image::CompositeSourceOver, (void*)context);
    cairo_destroy(context);
    
    context = cairo_create(finalSurface);
    cairo_set_source_surface(context, surface, 0, 0);
    cairo_rectangle(context, 0, 0, rect.right, rect.bottom);
    cairo_fill(context);
    cairo_destroy(context);

    cairo_surface_destroy(surface);
    cairo_surface_destroy(finalSurface);   
}


// ImageView printing

BOOL ImageView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void ImageView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void ImageView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// ImageView diagnostics

#ifdef _DEBUG
void ImageView::AssertValid() const
{
	CView::AssertValid();
}

void ImageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

ImageDocument* ImageView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(ImageDocument)));
	return (ImageDocument*)m_pDocument;
}
#endif //_DEBUG


// ImageView message handlers
