
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GEOMETRY

#include "wx/affinematrix2d.h"
#include "wx/math.h"

void wxAffineMatrix2D::Set(const wxMatrix2D &mat2D, const wxPoint2DDouble &tr)
{
    m_11 = mat2D.m_11;
    m_12 = mat2D.m_12;
    m_21 = mat2D.m_21;
    m_22 = mat2D.m_22;
    m_tx = tr.m_x;
    m_ty = tr.m_y;
}

void wxAffineMatrix2D::Get(wxMatrix2D *mat2D, wxPoint2DDouble *tr) const
{
    mat2D->m_11 = m_11;
    mat2D->m_12 = m_12;
    mat2D->m_21 = m_21;
    mat2D->m_22 = m_22;

    if ( tr )
    {
        tr->m_x = m_tx;
        tr->m_y = m_ty;
    }
}

void wxAffineMatrix2D::Concat(const wxAffineMatrix2DBase &t)
{
    wxMatrix2D mat;
    wxPoint2DDouble tr;
    t.Get(&mat, &tr);

    m_tx += tr.m_x*m_11 + tr.m_y*m_21;
    m_ty += tr.m_x*m_12 + tr.m_y*m_22;
    wxDouble e11 = mat.m_11*m_11 + mat.m_12*m_21;
    wxDouble e12 = mat.m_11*m_12 + mat.m_12*m_22;
    wxDouble e21 = mat.m_21*m_11 + mat.m_22*m_21;
    m_22 = mat.m_21*m_12 + mat.m_22*m_22;
    m_11 = e11;
    m_12 = e12;
    m_21 = e21;
}

bool wxAffineMatrix2D::Invert()
{
    const wxDouble det = m_11*m_22 - m_12*m_21;

    if ( !det )
        return false;

    wxDouble ex = (m_21*m_ty - m_22*m_tx) / det;
    m_ty = (-m_11*m_ty + m_12*m_tx) / det;
    m_tx = ex;
    wxDouble e11 = m_22 / det;
    m_12 = -m_12 / det;
    m_21 = -m_21 / det;
    m_22 = m_11 / det;
    m_11 = e11;

    return true;
}

bool wxAffineMatrix2D::IsEqual(const wxAffineMatrix2DBase& t) const
{
    wxMatrix2D mat;
    wxPoint2DDouble tr;
    t.Get(&mat, &tr);

    return m_11 == mat.m_11 && m_12 == mat.m_12 &&
           m_21 == mat.m_21 && m_22 == mat.m_22 &&
           m_tx == tr.m_x && m_ty == tr.m_y;
}


void wxAffineMatrix2D::Translate(wxDouble dx, wxDouble dy)
{
    m_tx += m_11 * dx + m_21 * dy;
    m_ty += m_12 * dx + m_22 * dy;
}

void wxAffineMatrix2D::Scale(wxDouble xScale, wxDouble yScale)
{
    m_11 *= xScale;
    m_12 *= xScale;
    m_21 *= yScale;
    m_22 *= yScale;
}

void wxAffineMatrix2D::Rotate(wxDouble cRadians)
{
    wxDouble c = cos(cRadians);
    wxDouble s = sin(cRadians);

    wxDouble e11 = c*m_11 + s*m_21;
    wxDouble e12 = c*m_12 + s*m_22;
    m_21 = c*m_21 - s*m_11;
    m_22 = c*m_22 - s*m_12;
    m_11 = e11;
    m_12 = e12;
}


wxPoint2DDouble
wxAffineMatrix2D::DoTransformPoint(const wxPoint2DDouble& src) const
{
    if ( IsIdentity() )
        return src;

    return wxPoint2DDouble(src.m_x * m_11 + src.m_y * m_21 + m_tx,
                           src.m_x * m_12 + src.m_y * m_22 + m_ty);
}

wxPoint2DDouble
wxAffineMatrix2D::DoTransformDistance(const wxPoint2DDouble& src) const
{
    if ( IsIdentity() )
        return src;

    return wxPoint2DDouble(src.m_x * m_11 + src.m_y * m_21,
                           src.m_x * m_12 + src.m_y * m_22);
}

bool wxAffineMatrix2D::IsIdentity() const
{
    return m_11 == 1 && m_12 == 0 &&
           m_21 == 0 && m_22 == 1 &&
           m_tx == 0 && m_ty == 0;
}

#endif 