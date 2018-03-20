

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/region.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
#endif


typedef struct Box
{
    wxCoord x1, x2, y1, y2;
} Box, BOX, BoxRec, *BoxPtr;

typedef struct REGION *Region;

struct REGION
{
public:
        REGION() {}

    REGION(const wxRect& rect)
    {
        rects = &extents;
        numRects = 1;
        extents.x1 = rect.x;
        extents.y1 = rect.y;
        extents.x2 = rect.x + rect.width;
        extents.y2 = rect.y + rect.height;
        size = 1;
    }

    BoxPtr GetBox(int i)
    {
        return i < numRects ? rects + i : NULL;
    }

        static bool XClipBox(
        Region r,
        wxRect *rect);
    static bool XOffsetRegion(
        register Region pRegion,
        register int x,
        register int y);
    static bool XIntersectRegion(
        Region reg1,
        Region reg2,          
        register Region newReg);               
    static bool XUnionRegion(
        Region reg1,
        Region reg2,             
        Region newReg);                  
    static bool XSubtractRegion(
        Region regM,
        Region regS,
        register Region regD);
    static bool XXorRegion(Region sra, Region srb, Region dr);
    static bool XEmptyRegion(
        Region r);
    static bool XEqualRegion(Region r1, Region r2);
    static bool XPointInRegion(
        Region pRegion,
        int x, int y);
    static wxRegionContain XRectInRegion(
        register Region region,
        int rx, int ry,
        unsigned int rwidth, unsigned int rheight);

protected:
    static Region XCreateRegion(void);
    static void miSetExtents (
        Region pReg);
    static bool XDestroyRegion(Region r);
    static int miIntersectO (
        register Region pReg,
        register BoxPtr r1,
        BoxPtr r1End,
        register BoxPtr r2,
        BoxPtr r2End,
        wxCoord y1,
        wxCoord y2);
    static void miRegionCopy(
        register Region dstrgn,
        register Region rgn);
    static int miCoalesce(
        register Region pReg, 
        int prevStart, 
        int curStart); 
    static void miRegionOp(
        register Region newReg, 
        Region reg1, 
        Region reg2, 
        int (*overlapFunc)(
            register Region     pReg,
            register BoxPtr     r1,
            BoxPtr              r1End,
            register BoxPtr     r2,
            BoxPtr              r2End,
            wxCoord             y1,
            wxCoord             y2), 
        int (*nonOverlap1Func)(
            register Region     pReg,
            register BoxPtr     r,
            BoxPtr              rEnd,
            register wxCoord    y1,
            register wxCoord    y2), 
        int (*nonOverlap2Func)(
            register Region     pReg,
            register BoxPtr     r,
            BoxPtr              rEnd,
            register wxCoord    y1,
            register wxCoord    y2)); 
    static int miUnionNonO (
        register Region pReg,
        register BoxPtr r,
        BoxPtr rEnd,
        register wxCoord y1,
        register wxCoord y2);
    static int miUnionO (
        register Region pReg,
        register BoxPtr r1,
        BoxPtr r1End,
        register BoxPtr r2,
        BoxPtr r2End,
        register wxCoord y1,
        register wxCoord y2);
    static int miSubtractNonO1 (
        register Region pReg,
        register BoxPtr r,
        BoxPtr rEnd,
        register wxCoord y1,
        register wxCoord y2);
    static int miSubtractO (
        register Region pReg,
        register BoxPtr r1,
        BoxPtr r1End,
        register BoxPtr r2,
        BoxPtr r2End,
        register wxCoord y1,
        register wxCoord y2);
protected:
    long size;
    long numRects;
    Box *rects;
    Box extents;
};


class wxRegionRefData : public wxGDIRefData,
                        public REGION
{
public:
    wxRegionRefData()
        : wxGDIRefData(),
          REGION()
    {
        size = 1;
        numRects = 0;
        rects = ( BOX * )malloc( (unsigned) sizeof( BOX ));
        extents.x1 = 0;
        extents.x2 = 0;
        extents.y1 = 0;
        extents.y2 = 0;
    }

    wxRegionRefData(const wxPoint& topLeft, const wxPoint& bottomRight)
        : wxGDIRefData(),
          REGION()
    {
        rects = (BOX*)malloc(sizeof(BOX));
        size = 1;
        numRects = 1;
        extents.x1 = topLeft.x;
        extents.y1 = topLeft.y;
        extents.x2 = bottomRight.x;
        extents.y2 = bottomRight.y;
        *rects = extents;
    }

    wxRegionRefData(const wxRect& rect)
        : wxGDIRefData(),
          REGION(rect)
    {
        rects = (BOX*)malloc(sizeof(BOX));
        *rects = extents;
    }

    wxRegionRefData(const wxRegionRefData& refData)
        : wxGDIRefData(),
          REGION()
    {
        size = refData.size;
        numRects = refData.numRects;
        rects = (Box*)malloc(numRects*sizeof(Box));
        memcpy(rects, refData.rects, numRects*sizeof(Box));
        extents = refData.extents;
    }

    virtual ~wxRegionRefData()
    {
        free(rects);
    }

private:
        wxRegionRefData(const REGION&);
};


#define M_REGIONDATA ((wxRegionRefData *)m_refData)
#define M_REGIONDATA_OF(rgn) ((wxRegionRefData *)(rgn.m_refData))


wxRegionGeneric::wxRegionGeneric()
{
}

wxRegionGeneric::~wxRegionGeneric()
{
}

wxRegionGeneric::wxRegionGeneric(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
    m_refData = new wxRegionRefData(wxRect(x,y,w,h));
}

wxRegionGeneric::wxRegionGeneric(const wxRect& rect)
{
    m_refData = new wxRegionRefData(rect);
}

wxRegionGeneric::wxRegionGeneric(const wxPoint& topLeft, const wxPoint& bottomRight)
{
    m_refData = new wxRegionRefData(topLeft, bottomRight);
}

wxRegionGeneric::wxRegionGeneric(const wxBitmap& bmp)
{
    wxFAIL_MSG("NOT IMPLEMENTED: wxRegionGeneric::wxRegionGeneric(const wxBitmap& bmp)");
}

wxRegionGeneric::wxRegionGeneric(size_t n, const wxPoint *points, wxPolygonFillMode fillStyle)
{
    wxFAIL_MSG("NOT IMPLEMENTED: wxRegionGeneric::wxRegionGeneric(size_t n, const wxPoint *points, wxPolygonFillMode fillStyle)");
}

wxRegionGeneric::wxRegionGeneric(const wxBitmap& bmp, const wxColour& transp, int tolerance)
{
    wxFAIL_MSG("NOT IMPLEMENTED: wxRegionGeneric::wxRegionGeneric(const wxBitmap& bmp, const wxColour& transp, int tolerance)");
}

void wxRegionGeneric::Clear()
{
    UnRef();
    if (!m_refData)
        m_refData = new wxRegionRefData(wxRect(0,0,0,0));
}

wxGDIRefData *wxRegionGeneric::CreateGDIRefData() const
{
    return new wxRegionRefData;
}

wxGDIRefData *wxRegionGeneric::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxRegionRefData(*(wxRegionRefData *)data);
}

bool wxRegionGeneric::DoIsEqual(const wxRegion& region) const
{
    return REGION::XEqualRegion(M_REGIONDATA,M_REGIONDATA_OF(region));
}

bool wxRegionGeneric::DoGetBox(wxCoord& x, wxCoord& y, wxCoord&w, wxCoord &h) const
{
    if ( !m_refData )
        return false;

    wxRect rect;
    REGION::XClipBox(M_REGIONDATA,&rect);
    x = rect.x;
    y = rect.y;
    w = rect.width;
    h = rect.height;
    return true;
}


bool wxRegionGeneric::DoUnionWithRect(const wxRect& rect)
{
    if ( rect.IsEmpty() )
    {
                return true;
    }

    AllocExclusive();
    REGION region(rect);
    return REGION::XUnionRegion(&region,M_REGIONDATA,M_REGIONDATA);
}

bool wxRegionGeneric::DoUnionWithRegion(const wxRegion& region)
{
    AllocExclusive();
    return REGION::XUnionRegion(M_REGIONDATA_OF(region),M_REGIONDATA,M_REGIONDATA);
}

bool wxRegionGeneric::DoIntersect(const wxRegion& region)
{
    AllocExclusive();
    return REGION::XIntersectRegion(M_REGIONDATA_OF(region),M_REGIONDATA,M_REGIONDATA);
}

bool wxRegionGeneric::DoSubtract(const wxRegion& region)
{
    if ( region.IsEmpty() )
    {
                return true;
    }

    return REGION::XSubtractRegion(M_REGIONDATA_OF(region),M_REGIONDATA,M_REGIONDATA);
}

bool wxRegionGeneric::DoXor(const wxRegion& region)
{
    AllocExclusive();
    return REGION::XXorRegion(M_REGIONDATA_OF(region),M_REGIONDATA,M_REGIONDATA);
}

bool wxRegionGeneric::DoOffset(wxCoord x, wxCoord y)
{
    AllocExclusive();
    return REGION::XOffsetRegion(M_REGIONDATA, x, y);
}


bool wxRegionGeneric::IsEmpty() const
{
    wxASSERT(m_refData);
    return REGION::XEmptyRegion(M_REGIONDATA);
}

wxRegionContain wxRegionGeneric::DoContainsPoint(wxCoord x, wxCoord y) const
{
    wxASSERT(m_refData);
    return REGION::XPointInRegion(M_REGIONDATA,x,y) ? wxInRegion : wxOutRegion;
}

wxRegionContain wxRegionGeneric::DoContainsRect(const wxRect& rect) const
{
    wxASSERT(m_refData);
    return REGION::XRectInRegion(M_REGIONDATA,rect.x,rect.y,rect.width,rect.height);
}


wxRegionIteratorGeneric::wxRegionIteratorGeneric()
{
    m_current = 0;
}

wxRegionIteratorGeneric::wxRegionIteratorGeneric(const wxRegionGeneric& region)
:   m_region(region)
{
    m_current = 0;
}

wxRegionIteratorGeneric::wxRegionIteratorGeneric(const wxRegionIteratorGeneric& iterator)
:   m_region(iterator.m_region)
{
    m_current = iterator.m_current;
}

void wxRegionIteratorGeneric::Reset(const wxRegionGeneric& region)
{
    m_region = region;
    m_current = 0;
}

bool wxRegionIteratorGeneric::HaveRects() const
{
    return M_REGIONDATA_OF(m_region)->GetBox(m_current);
}

wxRegionIteratorGeneric& wxRegionIteratorGeneric::operator++()
{
    ++m_current;
    return *this;
}

wxRegionIteratorGeneric wxRegionIteratorGeneric::operator++(int)
{
    wxRegionIteratorGeneric copy(*this);
    ++*this;
    return copy;
}

wxRect wxRegionIteratorGeneric::GetRect() const
{
    wxASSERT(m_region.m_refData);
    const Box *box = M_REGIONDATA_OF(m_region)->GetBox(m_current);
    wxASSERT(box);
    return wxRect
    (   box->x1
    ,   box->y1
    ,   box->x2 - box->x1
    ,   box->y2 - box->y1
    );
}

long wxRegionIteratorGeneric::GetX() const
{
    wxASSERT(m_region.m_refData);
    const Box *box = M_REGIONDATA_OF(m_region)->GetBox(m_current);
    wxASSERT(box);
    return box->x1;
}

long wxRegionIteratorGeneric::GetY() const
{
    wxASSERT(m_region.m_refData);
    const Box *box = M_REGIONDATA_OF(m_region)->GetBox(m_current);
    wxASSERT(box);
    return box->y1;
}

long wxRegionIteratorGeneric::GetW() const
{
    wxASSERT(m_region.m_refData);
    const Box *box = M_REGIONDATA_OF(m_region)->GetBox(m_current);
    wxASSERT(box);
    return box->x2 - box->x1;
}

long wxRegionIteratorGeneric::GetH() const
{
    wxASSERT(m_region.m_refData);
    const Box *box = M_REGIONDATA_OF(m_region)->GetBox(m_current);
    wxASSERT(box);
    return box->y2 - box->y1;
}

wxRegionIteratorGeneric::~wxRegionIteratorGeneric()
{
}






#define EXTENTCHECK(r1, r2) \
    ((r1)->x2 > (r2)->x1 && \
     (r1)->x1 < (r2)->x2 && \
     (r1)->y2 > (r2)->y1 && \
     (r1)->y1 < (r2)->y2)


#define MEMCHECK(reg, rect, firstrect){\
        if ((reg)->numRects >= ((reg)->size - 1)){\
          (firstrect) = (BOX *) realloc \
          ((char *)(firstrect), (unsigned) (2 * (sizeof(BOX)) * ((reg)->size)));\
          if ((firstrect) == 0)\
            return(0);\
          (reg)->size *= 2;\
          (rect) = &(firstrect)[(reg)->numRects];\
         }\
       }

#define EMPTY_REGION(pReg) pReg->numRects = 0

#define REGION_NOT_EMPTY(pReg) pReg->numRects

#define INBOX(r, x, y) \
      ( ( ((r).x2 >  x)) && \
        ( ((r).x1 <= x)) && \
        ( ((r).y2 >  y)) && \
        ( ((r).y1 <= y)) )




Region REGION::XCreateRegion(void)
{
    Region temp = new REGION;

    if (!temp)
        return (Region) NULL;

    temp->rects = ( BOX * )malloc( (unsigned) sizeof( BOX ));

    if (!temp->rects)
    {
        delete temp;
        return (Region) NULL;
    }
    temp->numRects = 0;
    temp->extents.x1 = 0;
    temp->extents.y1 = 0;
    temp->extents.x2 = 0;
    temp->extents.y2 = 0;
    temp->size = 1;
    return( temp );
}

bool REGION::XClipBox(Region r, wxRect *rect)
{
    rect->x = r->extents.x1;
    rect->y = r->extents.y1;
    rect->width = r->extents.x2 - r->extents.x1;
    rect->height = r->extents.y2 - r->extents.y1;
    return true;
}


void REGION::
miSetExtents (Region pReg)
{
    BoxPtr pBox, pBoxEnd, pExtents;

    if (pReg->numRects == 0)
    {
        pReg->extents.x1 = 0;
        pReg->extents.y1 = 0;
        pReg->extents.x2 = 0;
        pReg->extents.y2 = 0;
        return;
    }

    pExtents = &pReg->extents;
    pBox = pReg->rects;
    pBoxEnd = &pBox[pReg->numRects - 1];

    
    pExtents->x1 = pBox->x1;
    pExtents->y1 = pBox->y1;
    pExtents->x2 = pBoxEnd->x2;
    pExtents->y2 = pBoxEnd->y2;

    wxASSERT_LEVEL_2(pExtents->y1 < pExtents->y2);
    while (pBox <= pBoxEnd)
    {
        if (pBox->x1 < pExtents->x1)
        {
            pExtents->x1 = pBox->x1;
        }
        if (pBox->x2 > pExtents->x2)
        {
            pExtents->x2 = pBox->x2;
        }
        pBox++;
    }
    wxASSERT_LEVEL_2(pExtents->x1 < pExtents->x2);
}

bool REGION::
XDestroyRegion(
    Region r)
{
    free( (char *) r->rects );
    delete r;
    return true;
}



bool REGION::
XOffsetRegion(
    register Region pRegion,
    register int x,
    register int y)
{
    int nbox;
    BOX *pbox;

    pbox = pRegion->rects;
    nbox = pRegion->numRects;

    while(nbox--)
    {
        pbox->x1 += x;
        pbox->x2 += x;
        pbox->y1 += y;
        pbox->y2 += y;
        pbox++;
    }
    pRegion->extents.x1 += x;
    pRegion->extents.x2 += x;
    pRegion->extents.y1 += y;
    pRegion->extents.y2 += y;
    return 1;
}




int REGION::
miIntersectO (
    register Region     pReg,
    register BoxPtr     r1,
    BoxPtr              r1End,
    register BoxPtr     r2,
    BoxPtr              r2End,
    wxCoord             y1,
    wxCoord             y2)
{
    wxCoord    x1;
    wxCoord    x2;
    BoxPtr     pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

    while ((r1 != r1End) && (r2 != r2End))
    {
        x1 = wxMax(r1->x1,r2->x1);
        x2 = wxMin(r1->x2,r2->x2);

        
        if (x1 < x2)
        {
            wxASSERT_LEVEL_2(y1<y2);

            MEMCHECK(pReg, pNextRect, pReg->rects);
            pNextRect->x1 = x1;
            pNextRect->y1 = y1;
            pNextRect->x2 = x2;
            pNextRect->y2 = y2;
            pReg->numRects += 1;
            pNextRect++;
            wxASSERT_LEVEL_2(pReg->numRects <= pReg->size);
        }

        
        if (r1->x2 < r2->x2)
        {
            r1++;
        }
        else if (r2->x2 < r1->x2)
        {
            r2++;
        }
        else
        {
            r1++;
            r2++;
        }
    }
    return 0; 
}

bool REGION::
XIntersectRegion(
    Region reg1,
    Region reg2, 
    register Region newReg) 
{
   
    if ( (!(reg1->numRects)) || (!(reg2->numRects))  ||
        (!EXTENTCHECK(&reg1->extents, &reg2->extents)))
        newReg->numRects = 0;
    else
        miRegionOp (newReg, reg1, reg2,
                    miIntersectO, NULL, NULL);

    
    miSetExtents(newReg);
    return 1;
}

void REGION::
miRegionCopy(
    register Region dstrgn,
    register Region rgn)

{
    if (dstrgn != rgn) 
    {
        if (dstrgn->size < rgn->numRects)
        {
            if (dstrgn->rects)
            {
                BOX *prevRects = dstrgn->rects;

                dstrgn->rects = (BOX *)
                       realloc((char *) dstrgn->rects,
                               (unsigned) rgn->numRects * (sizeof(BOX)));
                if (!dstrgn->rects)
                {
                    free(prevRects);
                    return;
                }
            }
            dstrgn->size = rgn->numRects;
        }
        dstrgn->numRects = rgn->numRects;
        dstrgn->extents.x1 = rgn->extents.x1;
        dstrgn->extents.y1 = rgn->extents.y1;
        dstrgn->extents.x2 = rgn->extents.x2;
        dstrgn->extents.y2 = rgn->extents.y2;

        memcpy((char *) dstrgn->rects, (char *) rgn->rects,
                (int) (rgn->numRects * sizeof(BOX)));
    }
}





int REGION::
miCoalesce(
    register Region pReg,     
    int prevStart,            
    int curStart)             
{
    BoxPtr pPrevBox;          
    BoxPtr pCurBox;           
    BoxPtr pRegEnd;           
    int         curNumRects;  
    int        prevNumRects;  
    int              bandY1;  

    pRegEnd = &pReg->rects[pReg->numRects];

    pPrevBox = &pReg->rects[prevStart];
    prevNumRects = curStart - prevStart;

    
    pCurBox = &pReg->rects[curStart];
    bandY1 = pCurBox->y1;
    for (curNumRects = 0;
         (pCurBox != pRegEnd) && (pCurBox->y1 == bandY1);
         curNumRects++)
    {
        pCurBox++;
    }

    if (pCurBox != pRegEnd)
    {
        
        pRegEnd--;
        while (pRegEnd[-1].y1 == pRegEnd->y1)
        {
            pRegEnd--;
        }
        curStart = pRegEnd - pReg->rects;
        pRegEnd = pReg->rects + pReg->numRects;
    }

    if ((curNumRects == prevNumRects) && (curNumRects != 0))
    {
        pCurBox -= curNumRects;
        
        if (pPrevBox->y2 == pCurBox->y1)
        {
        
            do
            {
                if ((pPrevBox->x1 != pCurBox->x1) ||
                    (pPrevBox->x2 != pCurBox->x2))
                {
                    
                    return (curStart);
                }
                pPrevBox++;
                pCurBox++;
                prevNumRects -= 1;
            } while (prevNumRects != 0);

            pReg->numRects -= curNumRects;
            pCurBox -= curNumRects;
            pPrevBox -= curNumRects;

            
            do
            {
                pPrevBox->y2 = pCurBox->y2;
                pPrevBox++;
                pCurBox++;
                curNumRects -= 1;
            } while (curNumRects != 0);

            
            if (pCurBox == pRegEnd)
            {
                curStart = prevStart;
            }
            else
            {
                do
                {
                    *pPrevBox++ = *pCurBox++;
                } while (pCurBox != pRegEnd);
            }

        }
    }
    return (curStart);
}



void REGION::
miRegionOp(
    register Region         newReg,                              
    Region                  reg1,                                
    Region                  reg2,                                
    int                      (*overlapFunc)(
        register Region     pReg,
        register BoxPtr     r1,
        BoxPtr              r1End,
        register BoxPtr     r2,
        BoxPtr              r2End,
        wxCoord               y1,
        wxCoord               y2),              
    int                      (*nonOverlap1Func)(
        register Region     pReg,
        register BoxPtr     r,
        BoxPtr              rEnd,
        register wxCoord      y1,
        register wxCoord      y2),              
    int                      (*nonOverlap2Func)(
        register Region     pReg,
        register BoxPtr     r,
        BoxPtr              rEnd,
        register wxCoord      y1,
        register wxCoord      y2))              
{
    BoxPtr                 r1; 
    BoxPtr                 r2; 
    BoxPtr              r1End; 
    BoxPtr              r2End; 
    wxCoord              ybot; 
    wxCoord              ytop; 
    BoxPtr           oldRects; 
    int              prevBand; 
    int               curBand; 
    BoxPtr                r1BandEnd; 
    BoxPtr                r2BandEnd; 
    wxCoord               top; 
    wxCoord               bot; 

    
    r1 = reg1->rects;
    r2 = reg2->rects;
    r1End = r1 + reg1->numRects;
    r2End = r2 + reg2->numRects;

    oldRects = newReg->rects;

    EMPTY_REGION(newReg);

    
    newReg->size = wxMax(reg1->numRects,reg2->numRects) * 2;

    newReg->rects = (BoxPtr)malloc((unsigned) (sizeof(BoxRec) * newReg->size));

    if (!newReg->rects)
    {
        newReg->size = 0;
        return;
    }

    
    if (reg1->extents.y1 < reg2->extents.y1)
        ybot = reg1->extents.y1;
    else
        ybot = reg2->extents.y1;

    
    prevBand = 0;

    do
    {
        curBand = newReg->numRects;

        
        r1BandEnd = r1;
        while ((r1BandEnd != r1End) && (r1BandEnd->y1 == r1->y1))
        {
            r1BandEnd++;
        }

        r2BandEnd = r2;
        while ((r2BandEnd != r2End) && (r2BandEnd->y1 == r2->y1))
        {
            r2BandEnd++;
        }

        
        if (r1->y1 < r2->y1)
        {
            top = wxMax(r1->y1,ybot);
            bot = wxMin(r1->y2,r2->y1);

            if ((top != bot) && (nonOverlap1Func != NULL))
            {
                (* nonOverlap1Func) (newReg, r1, r1BandEnd, top, bot);
            }

            ytop = r2->y1;
        }
        else if (r2->y1 < r1->y1)
        {
            top = wxMax(r2->y1,ybot);
            bot = wxMin(r2->y2,r1->y1);

            if ((top != bot) && (nonOverlap2Func != NULL))
            {
                (* nonOverlap2Func) (newReg, r2, r2BandEnd, top, bot);
            }

            ytop = r1->y1;
        }
        else
        {
            ytop = r1->y1;
        }

        
        if (newReg->numRects != curBand)
        {
            prevBand = miCoalesce (newReg, prevBand, curBand);
        }

        
        ybot = wxMin(r1->y2, r2->y2);
        curBand = newReg->numRects;
        if (ybot > ytop)
        {
            (* overlapFunc) (newReg, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);

        }

        if (newReg->numRects != curBand)
        {
            prevBand = miCoalesce (newReg, prevBand, curBand);
        }

        
        if (r1->y2 == ybot)
        {
            r1 = r1BandEnd;
        }
        if (r2->y2 == ybot)
        {
            r2 = r2BandEnd;
        }
    } while ((r1 != r1End) && (r2 != r2End));

    
    curBand = newReg->numRects;
    if (r1 != r1End)
    {
        if (nonOverlap1Func != NULL)
        {
            do
            {
                r1BandEnd = r1;
                while ((r1BandEnd < r1End) && (r1BandEnd->y1 == r1->y1))
                {
                    r1BandEnd++;
                }
                (* nonOverlap1Func) (newReg, r1, r1BandEnd,
                                     wxMax(r1->y1,ybot), r1->y2);
                r1 = r1BandEnd;
            } while (r1 != r1End);
        }
    }
    else if ((r2 != r2End) && (nonOverlap2Func != NULL))
    {
        do
        {
            r2BandEnd = r2;
            while ((r2BandEnd < r2End) && (r2BandEnd->y1 == r2->y1))
            {
                 r2BandEnd++;
            }
            (* nonOverlap2Func) (newReg, r2, r2BandEnd,
                                wxMax(r2->y1,ybot), r2->y2);
            r2 = r2BandEnd;
        } while (r2 != r2End);
    }

    if (newReg->numRects != curBand)
    {
        (void) miCoalesce (newReg, prevBand, curBand);
    }

    
    if (newReg->numRects < (newReg->size >> 1))
    {
        if (REGION_NOT_EMPTY(newReg))
        {
            BoxPtr prev_rects = newReg->rects;
            newReg->size = newReg->numRects;
            newReg->rects = (BoxPtr) realloc ((char *) newReg->rects,
                                   (unsigned) (sizeof(BoxRec) * newReg->size));
            if (! newReg->rects)
                newReg->rects = prev_rects;
        }
        else
        {
            
            newReg->size = 1;
            free((char *) newReg->rects);
            newReg->rects = (BoxPtr) malloc(sizeof(BoxRec));
        }
    }
    free ((char *) oldRects);
    return;
}





int REGION::
miUnionNonO (
    register Region        pReg,
    register BoxPtr        r,
    BoxPtr                 rEnd,
    register wxCoord       y1,
    register wxCoord       y2)
{
    BoxPtr pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

    wxASSERT_LEVEL_2(y1 < y2);

    while (r != rEnd)
    {
        wxASSERT_LEVEL_2(r->x1 < r->x2);
        MEMCHECK(pReg, pNextRect, pReg->rects);
        pNextRect->x1 = r->x1;
        pNextRect->y1 = y1;
        pNextRect->x2 = r->x2;
        pNextRect->y2 = y2;
        pReg->numRects += 1;
        pNextRect++;

        wxASSERT_LEVEL_2(pReg->numRects<=pReg->size);
        r++;
    }
    return 0;        
}





int REGION::
miUnionO (
    register Region        pReg,
    register BoxPtr        r1,
    BoxPtr                 r1End,
    register BoxPtr        r2,
    BoxPtr                 r2End,
    register wxCoord       y1,
    register wxCoord       y2)
{
    BoxPtr pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

#define MERGERECT(r) \
    if ((pReg->numRects != 0) &&  \
        (pNextRect[-1].y1 == y1) &&  \
        (pNextRect[-1].y2 == y2) &&  \
        (pNextRect[-1].x2 >= r->x1))  \
    {  \
        if (pNextRect[-1].x2 < r->x2)  \
        {  \
            pNextRect[-1].x2 = r->x2;  \
            wxASSERT_LEVEL_2(pNextRect[-1].x1<pNextRect[-1].x2); \
        }  \
    }  \
    else  \
    {  \
        MEMCHECK(pReg, pNextRect, pReg->rects);  \
        pNextRect->y1 = y1;  \
        pNextRect->y2 = y2;  \
        pNextRect->x1 = r->x1;  \
        pNextRect->x2 = r->x2;  \
        pReg->numRects += 1;  \
        pNextRect += 1;  \
    }  \
    wxASSERT_LEVEL_2(pReg->numRects<=pReg->size);\
    r++;

    wxASSERT_LEVEL_2 (y1<y2);
    while ((r1 != r1End) && (r2 != r2End))
    {
        if (r1->x1 < r2->x1)
        {
            MERGERECT(r1);
        }
        else
        {
            MERGERECT(r2);
        }
    }

    if (r1 != r1End)
    {
        do
        {
            MERGERECT(r1);
        } while (r1 != r1End);
    }
    else while (r2 != r2End)
    {
        MERGERECT(r2);
    }
    return 0;        
}

bool REGION::
XUnionRegion(
    Region           reg1,
    Region          reg2,             
    Region           newReg)                  
{
    

    
    if ( (reg1 == reg2) || (!(reg1->numRects)) )
    {
        if (newReg != reg2)
            miRegionCopy(newReg, reg2);
        return 1;
    }

    
    if (!(reg2->numRects))
    {
        if (newReg != reg1)
            miRegionCopy(newReg, reg1);
        return 1;
    }

    
    if ((reg1->numRects == 1) &&
        (reg1->extents.x1 <= reg2->extents.x1) &&
        (reg1->extents.y1 <= reg2->extents.y1) &&
        (reg1->extents.x2 >= reg2->extents.x2) &&
        (reg1->extents.y2 >= reg2->extents.y2))
    {
        if (newReg != reg1)
            miRegionCopy(newReg, reg1);
        return 1;
    }

    
    if ((reg2->numRects == 1) &&
        (reg2->extents.x1 <= reg1->extents.x1) &&
        (reg2->extents.y1 <= reg1->extents.y1) &&
        (reg2->extents.x2 >= reg1->extents.x2) &&
        (reg2->extents.y2 >= reg1->extents.y2))
    {
        if (newReg != reg2)
            miRegionCopy(newReg, reg2);
        return 1;
    }

    miRegionOp (newReg, reg1, reg2, miUnionO,
                    miUnionNonO, miUnionNonO);

    newReg->extents.x1 = wxMin(reg1->extents.x1, reg2->extents.x1);
    newReg->extents.y1 = wxMin(reg1->extents.y1, reg2->extents.y1);
    newReg->extents.x2 = wxMax(reg1->extents.x2, reg2->extents.x2);
    newReg->extents.y2 = wxMax(reg1->extents.y2, reg2->extents.y2);

    return 1;
}





int REGION::
miSubtractNonO1 (
    register Region        pReg,
    register BoxPtr        r,
    BoxPtr                    rEnd,
    register wxCoord          y1,
    register wxCoord           y2)
{
    BoxPtr        pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

    wxASSERT_LEVEL_2(y1<y2);

    while (r != rEnd)
    {
        wxASSERT_LEVEL_2(r->x1<r->x2);
        MEMCHECK(pReg, pNextRect, pReg->rects);
        pNextRect->x1 = r->x1;
        pNextRect->y1 = y1;
        pNextRect->x2 = r->x2;
        pNextRect->y2 = y2;
        pReg->numRects += 1;
        pNextRect++;

        wxASSERT_LEVEL_2(pReg->numRects <= pReg->size);

        r++;
    }
    return 0;        
}



int REGION::
miSubtractO (
    register Region        pReg,
    register BoxPtr        r1,
    BoxPtr                    r1End,
    register BoxPtr        r2,
    BoxPtr                    r2End,
    register wxCoord          y1,
    register wxCoord          y2)
{
    BoxPtr        pNextRect;
    int          x1;

    x1 = r1->x1;

    wxASSERT_LEVEL_2(y1<y2);
    pNextRect = &pReg->rects[pReg->numRects];

    while ((r1 != r1End) && (r2 != r2End))
    {
        if (r2->x2 <= x1)
        {
            
            r2++;
        }
        else if (r2->x1 <= x1)
        {
            
            x1 = r2->x2;
            if (x1 >= r1->x2)
            {
                
                r1++;
                if (r1 != r1End)
                    x1 = r1->x1;
            }
            else
            {
                
                r2++;
            }
        }
        else if (r2->x1 < r1->x2)
        {
            
            wxASSERT_LEVEL_2(x1<r2->x1);
            MEMCHECK(pReg, pNextRect, pReg->rects);
            pNextRect->x1 = x1;
            pNextRect->y1 = y1;
            pNextRect->x2 = r2->x1;
            pNextRect->y2 = y2;
            pReg->numRects += 1;
            pNextRect++;

            wxASSERT_LEVEL_2(pReg->numRects<=pReg->size);

            x1 = r2->x2;
            if (x1 >= r1->x2)
            {
                
                r1++;
                if (r1 != r1End)
                    x1 = r1->x1;
            }
            else
            {
                
                r2++;
            }
        }
        else
        {
            
            if (r1->x2 > x1)
            {
                MEMCHECK(pReg, pNextRect, pReg->rects);
                pNextRect->x1 = x1;
                pNextRect->y1 = y1;
                pNextRect->x2 = r1->x2;
                pNextRect->y2 = y2;
                pReg->numRects += 1;
                pNextRect++;
                wxASSERT_LEVEL_2(pReg->numRects<=pReg->size);
            }
            r1++;
            if (r1 != r1End)
                x1 = r1->x1;
        }
    }

    
    while (r1 != r1End)
    {
        wxASSERT_LEVEL_2(x1<r1->x2);
        MEMCHECK(pReg, pNextRect, pReg->rects);
        pNextRect->x1 = x1;
        pNextRect->y1 = y1;
        pNextRect->x2 = r1->x2;
        pNextRect->y2 = y2;
        pReg->numRects += 1;
        pNextRect++;

        wxASSERT_LEVEL_2(pReg->numRects<=pReg->size);

        r1++;
        if (r1 != r1End)
        {
            x1 = r1->x1;
        }
    }
    return 0;        
}



bool REGION::XSubtractRegion(Region regM, Region regS, register Region regD)
{
   
    if ( (!(regM->numRects)) || (!(regS->numRects))  ||
        (!EXTENTCHECK(&regM->extents, &regS->extents)) )
    {
        miRegionCopy(regD, regM);
        return true;
    }

    miRegionOp (regD, regM, regS, miSubtractO,
                    miSubtractNonO1, NULL);

    
    miSetExtents (regD);
    return true;
}

bool REGION::XXorRegion(Region sra, Region srb, Region dr)
{
    Region tra = XCreateRegion();

    wxCHECK_MSG( tra, false, wxT("region not created") );

    Region trb = XCreateRegion();

    wxCHECK_MSG( trb, false, wxT("region not created") );

    (void) XSubtractRegion(sra,srb,tra);
    (void) XSubtractRegion(srb,sra,trb);
    (void) XUnionRegion(tra,trb,dr);
    XDestroyRegion(tra);
    XDestroyRegion(trb);
    return 0;
}


bool REGION::XEmptyRegion(Region r)
{
    if( r->numRects == 0 ) return true;
    else  return false;
}


bool REGION::XEqualRegion(Region r1, Region r2)
{
    int i;

    if( r1->numRects != r2->numRects ) return false;
    else if( r1->numRects == 0 ) return true;
    else if ( r1->extents.x1 != r2->extents.x1 ) return false;
    else if ( r1->extents.x2 != r2->extents.x2 ) return false;
    else if ( r1->extents.y1 != r2->extents.y1 ) return false;
    else if ( r1->extents.y2 != r2->extents.y2 ) return false;
    else for( i=0; i < r1->numRects; i++ ) {
            if ( r1->rects[i].x1 != r2->rects[i].x1 ) return false;
            else if ( r1->rects[i].x2 != r2->rects[i].x2 ) return false;
            else if ( r1->rects[i].y1 != r2->rects[i].y1 ) return false;
            else if ( r1->rects[i].y2 != r2->rects[i].y2 ) return false;
    }
    return true;
}

bool REGION::XPointInRegion(Region pRegion, int x, int y)
{
    int i;

    if (pRegion->numRects == 0)
        return false;
    if (!INBOX(pRegion->extents, x, y))
        return false;
    for (i=0; i<pRegion->numRects; i++)
    {
        if (INBOX (pRegion->rects[i], x, y))
            return true;
    }
    return false;
}

wxRegionContain REGION::XRectInRegion(register Region region,
                                      int rx, int ry,
                                      unsigned int rwidth,
                                      unsigned int rheight)
{
    BoxPtr pbox;
    BoxPtr pboxEnd;
    Box rect;
    BoxPtr prect = &rect;
    int      partIn, partOut;

    prect->x1 = rx;
    prect->y1 = ry;
    prect->x2 = rwidth + rx;
    prect->y2 = rheight + ry;

    
    if ((region->numRects == 0) || !EXTENTCHECK(&region->extents, prect))
        return(wxOutRegion);

    partOut = false;
    partIn = false;

    
    for (pbox = region->rects, pboxEnd = pbox + region->numRects;
         pbox < pboxEnd;
         pbox++)
    {

        if (pbox->y2 <= ry)
           continue;        

        if (pbox->y1 > ry)
        {
           partOut = true;        
           if (partIn || (pbox->y1 >= prect->y2))
              break;
           ry = pbox->y1;        
        }

        if (pbox->x2 <= rx)
           continue;                

        if (pbox->x1 > rx)
        {
           partOut = true;        
           if (partIn)
              break;
        }

        if (pbox->x1 < prect->x2)
        {
            partIn = true;        
            if (partOut)
               break;
        }

        if (pbox->x2 >= prect->x2)
        {
           ry = pbox->y2;        
           if (ry >= prect->y2)
              break;
           rx = prect->x1;        
        } else
        {
            
            break;
        }

    }

    return(partIn ? ((ry < prect->y2) ? wxPartRegion : wxInRegion) :
                wxOutRegion);
}
