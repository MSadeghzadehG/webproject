
#include "wx/wxprec.h"

#if wxOSX_USE_COCOA_OR_CARBON

#include "wx/region.h"

#ifndef WX_PRECOMP
    #include "wx/gdicmn.h"
    #include "wx/dcmemory.h"
#endif

#include "wx/osx/private.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxRegion, wxGDIObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxRegionIterator, wxObject);

#define OSX_USE_SCANLINES 1


class WXDLLEXPORT wxRegionRefData : public wxGDIRefData
{
public:
    wxRegionRefData()
    {
        m_macRgn.reset( HIShapeCreateMutable() );
    }

    wxRegionRefData(wxCFRef<HIShapeRef> &region)
    {
        m_macRgn.reset( HIShapeCreateMutableCopy(region) );
    }

    wxRegionRefData(long x, long y, long w, long h)
    {
        CGRect r = CGRectMake(x,y,w,h);
        wxCFRef<HIShapeRef> rect(HIShapeCreateWithRect(&r));
        m_macRgn.reset( HIShapeCreateMutableCopy(rect) );
    }

    wxRegionRefData(const wxRegionRefData& data)
        : wxGDIRefData()
    {
        m_macRgn.reset( HIShapeCreateMutableCopy(data.m_macRgn) );
    }

    virtual ~wxRegionRefData()
    {
    }

    wxCFRef<HIMutableShapeRef> m_macRgn;
};

#define M_REGION (((wxRegionRefData*)m_refData)->m_macRgn)
#define OTHER_M_REGION(a) (((wxRegionRefData*)(a.m_refData))->m_macRgn)


wxRegion::wxRegion(WXHRGN hRegion )
{
    wxCFRef< HIShapeRef > shape( (HIShapeRef) hRegion );
    m_refData = new wxRegionRefData(shape);
}

wxRegion::wxRegion(long x, long y, long w, long h)
{
    m_refData = new wxRegionRefData(x , y , w , h );
}

wxRegion::wxRegion(const wxPoint& topLeft, const wxPoint& bottomRight)
{
    m_refData = new wxRegionRefData(topLeft.x , topLeft.y ,
                                    bottomRight.x - topLeft.x,
                                    bottomRight.y - topLeft.y);
}

wxRegion::wxRegion(const wxRect& rect)
{
    m_refData = new wxRegionRefData(rect.x , rect.y , rect.width , rect.height);
}

#if OSX_USE_SCANLINES









#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
int dx;       \
\
 \
if ((dy) != 0) { \
xStart = (x1); \
dx = (x2) - xStart; \
if (dx < 0) { \
m = dx / (dy); \
m1 = m - 1; \
incr1 = -2 * dx + 2 * (dy) * m1; \
incr2 = -2 * dx + 2 * (dy) * m; \
d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
} else { \
m = dx / (dy); \
m1 = m + 1; \
incr1 = 2 * dx - 2 * (dy) * m1; \
incr2 = 2 * dx - 2 * (dy) * m; \
d = -2 * m * (dy) + 2 * dx; \
} \
} \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
if (m1 > 0) { \
if (d > 0) { \
minval += m1; \
d += incr1; \
} \
else { \
minval += m; \
d += incr2; \
} \
} else {\
if (d >= 0) { \
minval += m1; \
d += incr1; \
} \
else { \
minval += m; \
d += incr2; \
} \
} \
}



typedef struct {
    int minor;         
    int d;           
    int m, m1;       
    int incr1, incr2; 
} BRESINFO;


#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
BRESINITPGON(dmaj, min1, min2, bres.minor, bres.d, \
bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
BRESINCRPGON(bres.d, bres.minor, bres.m, bres.m1, bres.incr1, bres.incr2)







#define CLOCKWISE          1
#define COUNTERCLOCKWISE  -1

typedef struct _EdgeTableEntry {
    int ymax;             
    BRESINFO bres;        
    struct _EdgeTableEntry *next;       
    struct _EdgeTableEntry *back;       
    struct _EdgeTableEntry *nextWETE;   
    int ClockWise;        
} EdgeTableEntry;


typedef struct _ScanLineList{
    int scanline;              
    EdgeTableEntry *edgelist;  
    struct _ScanLineList *next;  
} ScanLineList;


typedef struct {
    int ymax;                 
    int ymin;                 
    ScanLineList scanlines;   
} EdgeTable;



#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
    ScanLineList SLLs[SLLSPERBLOCK];
    struct _ScanLineListBlock *next;
} ScanLineListBlock;


#define NUMPTSTOBUFFER 200



#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
if (pAET->ymax == y) {           \
pPrevAET->next = pAET->next; \
pAET = pPrevAET->next; \
fixWAET = 1; \
if (pAET) \
pAET->back = pPrevAET; \
} \
else { \
BRESINCRPGONSTRUCT(pAET->bres); \
pPrevAET = pAET; \
pAET = pAET->next; \
} \
}



#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
if (pAET->ymax == y) {           \
pPrevAET->next = pAET->next; \
pAET = pPrevAET->next; \
if (pAET) \
pAET->back = pPrevAET; \
} \
else { \
BRESINCRPGONSTRUCT(pAET->bres); \
pPrevAET = pAET; \
pAET = pAET->next; \
} \
}



static bool miCreateETandAET(
                             int ,
                             const wxPoint * ,
                             EdgeTable * ,
                             EdgeTableEntry * ,
                             EdgeTableEntry * ,
                             ScanLineListBlock * 
                             );

static void miloadAET(
                      EdgeTableEntry * ,
                      EdgeTableEntry * 
                      );

static void micomputeWAET(
                          EdgeTableEntry * 
                          );

static int miInsertionSort(
                           EdgeTableEntry * 
                           );

static void miFreeStorage(
                          ScanLineListBlock * 
                          );




static bool
miInsertEdgeInET(EdgeTable *ET, EdgeTableEntry *ETE,  int scanline,
                 ScanLineListBlock **SLLBlock, int *iSLLBlock)
{
    EdgeTableEntry *start, *prev;
    ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;
    
    
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline))
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }
    
    
    if ((!pSLL) || (pSLL->scanline > scanline))
    {
        if (*iSLLBlock > SLLSPERBLOCK-1)
        {
            tmpSLLBlock =
            (ScanLineListBlock *)malloc(sizeof(ScanLineListBlock));
            if (!tmpSLLBlock)
                return FALSE;
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = (ScanLineListBlock *)NULL;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);
        
        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = (EdgeTableEntry *)NULL;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;
    
    
    prev = (EdgeTableEntry *)NULL;
    start = pSLL->edgelist;
    while (start && (start->bres.minor < ETE->bres.minor))
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;
    
    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
    return TRUE;
}



static bool
miCreateETandAET(int count, const wxPoint * pts, EdgeTable *ET, EdgeTableEntry *AET,
                 EdgeTableEntry *pETEs, ScanLineListBlock *pSLLBlock)
{
    const wxPoint* top, *bottom;
    const wxPoint* PrevPt, *CurrPt;
    int iSLLBlock = 0;
    
    int dy;
    
    if (count < 2)  return TRUE;
    
    
    AET->next = (EdgeTableEntry *)NULL;
    AET->back = (EdgeTableEntry *)NULL;
    AET->nextWETE = (EdgeTableEntry *)NULL;
    AET->bres.minor = INT_MIN;
    
    
    ET->scanlines.next = (ScanLineList *)NULL;
    ET->ymax = INT_MIN;
    ET->ymin = INT_MAX;
    pSLLBlock->next = (ScanLineListBlock *)NULL;
    
    PrevPt = &pts[count-1];
    
    
    while (count--)
    {
        CurrPt = pts++;
        
        
        if (PrevPt->y > CurrPt->y)
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = 0;
        }
        else
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = 1;
        }
        
        
        if (bottom->y != top->y)
        {
            pETEs->ymax = bottom->y-1;  
            
            
            dy = bottom->y - top->y;
            BRESINITPGONSTRUCT(dy, top->x, bottom->x, pETEs->bres);
            
            if (!miInsertEdgeInET(ET, pETEs, top->y, &pSLLBlock, &iSLLBlock))
            {
                miFreeStorage(pSLLBlock->next);
                return FALSE;
            }
            
            ET->ymax = wxMax(ET->ymax, PrevPt->y);
            ET->ymin = wxMin(ET->ymin, PrevPt->y);
            pETEs++;
        }
        
        PrevPt = CurrPt;
    }
    return TRUE;
}



static void
miloadAET(EdgeTableEntry *AET, EdgeTableEntry *ETEs)
{
    EdgeTableEntry *pPrevAET;
    EdgeTableEntry *tmp;
    
    pPrevAET = AET;
    AET = AET->next;
    while (ETEs)
    {
        while (AET && (AET->bres.minor < ETEs->bres.minor))
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;
        
        ETEs = tmp;
    }
}


static void
micomputeWAET(EdgeTableEntry *AET)
{
    EdgeTableEntry *pWETE;
    int inside = 1;
    int isInside = 0;
    
    AET->nextWETE = (EdgeTableEntry *)NULL;
    pWETE = AET;
    AET = AET->next;
    while (AET)
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;
        
        if ((!inside && !isInside) ||
            ( inside &&  isInside))
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = (EdgeTableEntry *)NULL;
}



static int
miInsertionSort(EdgeTableEntry *AET)
{
    EdgeTableEntry *pETEchase;
    EdgeTableEntry *pETEinsert;
    EdgeTableEntry *pETEchaseBackTMP;
    int changed = 0;
    
    AET = AET->next;
    while (AET)
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor > AET->bres.minor)
            pETEchase = pETEchase->back;
        
        AET = AET->next;
        if (pETEchase != pETEinsert)
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = 1;
        }
    }
    return(changed);
}


static void
miFreeStorage(ScanLineListBlock *pSLLBlock)
{
    ScanLineListBlock   *tmpSLLBlock;
    
    while (pSLLBlock) 
    {
        tmpSLLBlock = pSLLBlock->next;
        free(pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}



static bool
scanFillGeneralPoly( wxRegion* rgn,
                  int   count,              
                  const wxPoint *ptsIn,               
                  wxPolygonFillMode fillStyle
                  )
{
    EdgeTableEntry *pAET;  
    int y;                 
    int nPts = 0;          
    EdgeTableEntry *pWETE; 
    ScanLineList *pSLL;    
    wxPoint * ptsOut;      
    int *width;
    wxPoint FirstPoint[NUMPTSTOBUFFER]; 
    int FirstWidth[NUMPTSTOBUFFER];
    EdgeTableEntry *pPrevAET;       
    EdgeTable ET;                   
    EdgeTableEntry AET;             
    EdgeTableEntry *pETEs;          
    ScanLineListBlock SLLBlock;     
    int fixWAET = 0;
    
    if (count < 3)
        return(TRUE);
    
    if(!(pETEs = (EdgeTableEntry *)
         malloc(sizeof(EdgeTableEntry) * count)))
        return(FALSE);
    ptsOut = FirstPoint;
    width = FirstWidth;
    if (!miCreateETandAET(count, ptsIn, &ET, &AET, pETEs, &SLLBlock))
    {
        free(pETEs);
        return(FALSE);
    }
    pSLL = ET.scanlines.next;
    
    if (fillStyle == wxODDEVEN_RULE)
    {
        
        for (y = ET.ymin; y < ET.ymax; y++)
        {
            
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            
            
            while (pAET)
            {
                ptsOut->x = pAET->bres.minor;
                ptsOut++->y = y;
                *width++ = pAET->next->bres.minor - pAET->bres.minor;
                nPts++;
                
                
                if (nPts == NUMPTSTOBUFFER)
                {
                                                            
                    for ( int i = 0 ; i < nPts; ++i)
                    {
                        wxRect rect;
                        rect.y = FirstPoint[i].y;
                        rect.x = FirstPoint[i].x;
                        rect.height = 1;
                        rect.width = FirstWidth[i];
                        rgn->Union(rect);
                    }
                    ptsOut = FirstPoint;
                    width = FirstWidth;
                    nPts = 0;
                }
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y);
            }
            miInsertionSort(&AET);
        }
    }
    else      
    {
        
        for (y = ET.ymin; y < ET.ymax; y++)
        {
            
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                micomputeWAET(&AET);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            pWETE = pAET;
            
            
            while (pAET)
            {
                
                if (pWETE == pAET)
                {
                    ptsOut->x = pAET->bres.minor;
                    ptsOut++->y = y;
                    *width++ = pAET->nextWETE->bres.minor - pAET->bres.minor;
                    nPts++;
                    
                    
                    if (nPts == NUMPTSTOBUFFER)
                    {
                                                                        for ( int i = 0 ; i < nPts ; ++i)
                        {
                            wxRect rect;
                            rect.y = FirstPoint[i].y;
                            rect.x = FirstPoint[i].x;
                            rect.height = 1;
                            rect.width = FirstWidth[i];
                            rgn->Union(rect);
                        }
                        ptsOut = FirstPoint;
                        width  = FirstWidth;
                        nPts = 0;
                    }
                    
                    pWETE = pWETE->nextWETE;
                    while (pWETE != pAET)
                        EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
                    pWETE = pWETE->nextWETE;
                }
                EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
            }
            
            
            if (miInsertionSort(&AET) || fixWAET)
            {
                micomputeWAET(&AET);
                fixWAET = 0;
            }
        }
    }
    
    
            for ( int i = 0 ; i < nPts; ++i)
    {
        wxRect rect;
        rect.y = FirstPoint[i].y;
        rect.x = FirstPoint[i].x;
        rect.height = 1;
        rect.width = FirstWidth[i];
        rgn->Union(rect);
    }

    free(pETEs);
    miFreeStorage(SLLBlock.next);
    return(TRUE);
}

#endif

wxRegion::wxRegion(size_t n, const wxPoint *points, wxPolygonFillMode fillStyle)
{
         
    m_refData = new wxRegionRefData(); 
    
#if OSX_USE_SCANLINES
    scanFillGeneralPoly(this,n,points,fillStyle);
#else
    wxCoord mx = 0; 
    wxCoord my = 0; 
    wxPoint p; 
    size_t idx;     
     
        for (idx=0; idx<n; idx++) 
    { 
        wxPoint pt = points[idx]; 
        if (pt.x > mx) 
            mx = pt.x; 
        if (pt.y > my) 
            my = pt.y; 
    } 
 
        wxBitmap bmp(mx, my); 
    wxMemoryDC dc(bmp); 
    dc.SetBackground(*wxBLACK_BRUSH); 
    dc.Clear(); 
    dc.SetPen(*wxWHITE_PEN); 
    dc.SetBrush(*wxWHITE_BRUSH); 
    dc.DrawPolygon(n, (wxPoint*)points, 0, 0, fillStyle); 
    dc.SelectObject(wxNullBitmap); 
    bmp.SetMask(new wxMask(bmp, *wxBLACK)); 
 
        Union(bmp);
#endif
}

wxRegion::~wxRegion()
{
    }

wxGDIRefData *wxRegion::CreateGDIRefData() const
{
    return new wxRegionRefData;
}

wxGDIRefData *wxRegion::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxRegionRefData(*static_cast<const wxRegionRefData *>(data));
}


void wxRegion::Clear()
{
    UnRef();
}

bool wxRegion::DoOffset(wxCoord x, wxCoord y)
{
    wxCHECK_MSG( m_refData, false, wxT("invalid wxRegion") );

    if ( !x && !y )
                return true;

    AllocExclusive();

    verify_noerr( HIShapeOffset( M_REGION , x , y ) ) ;

    return true ;
}

bool wxRegion::DoUnionWithRect(const wxRect& rect)
{
    if ( !m_refData )
    {
        m_refData = new wxRegionRefData(rect.x , rect.y , rect.width , rect.height);
        return true;
    }
    
    AllocExclusive();
    
    CGRect r = CGRectMake(rect.x , rect.y , rect.width , rect.height);
    HIShapeUnionWithRect(M_REGION , &r);
    
    return true;
}

bool wxRegion::DoCombine(const wxRegion& region, wxRegionOp op)
{
    wxCHECK_MSG( region.IsOk(), false, wxT("invalid wxRegion") );

            if ( !m_refData )
    {
        switch ( op )
        {
            case wxRGN_COPY:
            case wxRGN_OR:
            case wxRGN_XOR:
                                *this = region;
                return true;

            case wxRGN_AND:
            case wxRGN_DIFF:
                                                return false;
        }

        wxFAIL_MSG( wxT("Unknown region operation") );
        return false;
    }

    AllocExclusive();

    switch (op)
    {
        case wxRGN_AND:
            verify_noerr( HIShapeIntersect( M_REGION , OTHER_M_REGION(region) , M_REGION ) );
            break ;

        case wxRGN_OR:
            verify_noerr( HIShapeUnion( M_REGION , OTHER_M_REGION(region) , M_REGION ) );
            break ;

        case wxRGN_XOR:
            {
                                wxCFRef< HIShapeRef > unionshape( HIShapeCreateUnion( M_REGION , OTHER_M_REGION(region) ) );
                wxCFRef< HIShapeRef > intersectionshape( HIShapeCreateIntersection( M_REGION , OTHER_M_REGION(region) ) );
                verify_noerr( HIShapeDifference( unionshape, intersectionshape, M_REGION ) );
            }
            break ;

        case wxRGN_DIFF:
            verify_noerr( HIShapeDifference( M_REGION , OTHER_M_REGION(region) , M_REGION ) ) ;
            break ;

        case wxRGN_COPY:
        default:
            M_REGION.reset( HIShapeCreateMutableCopy( OTHER_M_REGION(region) ) );
            break ;
    }

    return true;
}


bool wxRegion::DoIsEqual(const wxRegion& region) const
{
                wxRegion r(*this);
    r.Subtract(region);

    if ( !r.IsEmpty() )
        return false;

    wxRegion r2(region);
    r2.Subtract(*this);

    return r2.IsEmpty();
}

bool wxRegion::DoGetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const
{
    if (m_refData)
    {
        CGRect box ;
        HIShapeGetBounds( M_REGION , &box ) ;
        x = static_cast<int>(box.origin.x);
        y = static_cast<int>(box.origin.y);
        w = static_cast<int>(box.size.width);
        h = static_cast<int>(box.size.height);

        return true;
    }
    else
    {
        x = y = w = h = 0;

        return false;
    }
}

bool wxRegion::IsEmpty() const
{
    if ( m_refData )
        return HIShapeIsEmpty( M_REGION ) ;
    else
        return true ;
}

WXHRGN wxRegion::GetWXHRGN() const
{
    if ( !m_refData )
        return NULL;

    return M_REGION ;
}


wxRegionContain wxRegion::DoContainsPoint(wxCoord x, wxCoord y) const
{
    if (!m_refData)
        return wxOutRegion;

    CGPoint p = CGPointMake( x, y ) ;
    if (HIShapeContainsPoint( M_REGION , &p ) )
        return wxInRegion;

    return wxOutRegion;
}

wxRegionContain wxRegion::DoContainsRect(const wxRect& r) const
{
    if (!m_refData)
        return wxOutRegion;

    CGRect rect = CGRectMake(r.x,r.y,r.width,r.height);
    wxCFRef<HIShapeRef> rectshape(HIShapeCreateWithRect(&rect));
    wxCFRef<HIShapeRef> intersect(HIShapeCreateIntersection(rectshape,M_REGION));
    CGRect bounds;
    HIShapeGetBounds(intersect, &bounds);

    if ( HIShapeIsRectangular(intersect) && CGRectEqualToRect(rect,bounds) )
        return wxInRegion;
    else if ( HIShapeIsEmpty( intersect ) )
        return wxOutRegion;
    else
        return wxPartRegion;
}



wxRegionIterator::wxRegionIterator()
    : m_current(0), m_numRects(0), m_rects(NULL)
{
}

wxRegionIterator::~wxRegionIterator()
{
    wxDELETEA(m_rects);
}

wxRegionIterator::wxRegionIterator(const wxRegionIterator& iterator)
    : wxObject()
    , m_current(iterator.m_current)
    , m_numRects(0)
    , m_rects(NULL)
{
    SetRects(iterator.m_numRects, iterator.m_rects);
}

wxRegionIterator& wxRegionIterator::operator=(const wxRegionIterator& iterator)
{
    m_current  = iterator.m_current;
    SetRects(iterator.m_numRects, iterator.m_rects);

    return *this;
}


void wxRegionIterator::SetRects(long numRects, wxRect *rects)
{
    wxDELETEA(m_rects);

    if (rects && (numRects > 0))
    {
        int i;

        m_rects = new wxRect[numRects];
        for (i = 0; i < numRects; i++)
            m_rects[i] = rects[i];
    }

    m_numRects = numRects;
}


wxRegionIterator::wxRegionIterator(const wxRegion& region)
{
    m_rects = NULL;

    Reset(region);
}



class RegionToRectsCallbackData
{
public :
    wxRect* m_rects ;
    long m_current ;
};

OSStatus wxOSXRegionToRectsCounterCallback(
    int message, HIShapeRef WXUNUSED(region), const CGRect *WXUNUSED(rect), void *data )
{
    long *m_numRects = (long*) data ;
    if ( message == kHIShapeEnumerateInit )
    {
        (*m_numRects) = 0 ;
    }
    else if (message == kHIShapeEnumerateRect)
    {
        (*m_numRects) += 1 ;
    }

    return noErr;
}

OSStatus wxOSXRegionToRectsSetterCallback(
    int message, HIShapeRef WXUNUSED(region), const CGRect *rect, void *data )
{
    if (message == kHIShapeEnumerateRect)
    {
        RegionToRectsCallbackData *cb = (RegionToRectsCallbackData*) data ;
        cb->m_rects[cb->m_current++] = wxRect( rect->origin.x , rect->origin.y , rect->size.width , rect->size.height ) ;
    }

    return noErr;
}

void wxRegionIterator::Reset(const wxRegion& region)
{
    m_current = 0;
    m_region = region;

    wxDELETEA(m_rects);

    if (m_region.IsEmpty())
    {
        m_numRects = 0;
    }
    else
    {
#if 0
                        m_numRects = 1;
        m_rects = new wxRect[m_numRects];
        m_rects[0] = m_region.GetBox();
#endif
        OSStatus err = HIShapeEnumerate (OTHER_M_REGION(region), kHIShapeParseFromTopLeft, wxOSXRegionToRectsCounterCallback,
            (void*)&m_numRects);
        if (err == noErr)
        {
            m_rects = new wxRect[m_numRects];
            RegionToRectsCallbackData data ;
            data.m_rects = m_rects ;
            data.m_current = 0 ;
            HIShapeEnumerate( OTHER_M_REGION(region), kHIShapeParseFromTopLeft, wxOSXRegionToRectsSetterCallback,
                (void*)&data );
        }
        else
        {
            m_numRects = 0;
        }
    }
}


wxRegionIterator& wxRegionIterator::operator ++ ()
{
    if (m_current < m_numRects)
        ++m_current;

    return *this;
}


wxRegionIterator wxRegionIterator::operator ++ (int)
{
    wxRegionIterator previous(*this);

    if (m_current < m_numRects)
        ++m_current;

    return previous;
}

long wxRegionIterator::GetX() const
{
    if (m_current < m_numRects)
        return m_rects[m_current].x;

    return 0;
}

long wxRegionIterator::GetY() const
{
    if (m_current < m_numRects)
        return m_rects[m_current].y;

    return 0;
}

long wxRegionIterator::GetW() const
{
    if (m_current < m_numRects)
        return m_rects[m_current].width ;

    return 0;
}

long wxRegionIterator::GetH() const
{
    if (m_current < m_numRects)
        return m_rects[m_current].height;

    return 0;
}

#endif
