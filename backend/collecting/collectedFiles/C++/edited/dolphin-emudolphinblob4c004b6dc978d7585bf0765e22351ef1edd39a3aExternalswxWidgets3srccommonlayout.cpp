


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CONSTRAINTS

#include "wx/layout.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/utils.h"
    #include "wx/dialog.h"
    #include "wx/msgdlg.h"
    #include "wx/intl.h"
#endif


wxIMPLEMENT_DYNAMIC_CLASS(wxIndividualLayoutConstraint, wxObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxLayoutConstraints, wxObject);


inline void wxGetAsIs(wxWindowBase* win, int* w, int* h)
{
#if 1
        win->GetSize(w, h);
#endif

#if 0
        win->GetBestSize(w, h);
#endif

#if 0
        int sw, sh, bw, bh;
    win->GetSize(&sw, &sh);
    win->GetBestSize(&bw, &bh);
    if (w)
        *w = wxMax(sw, bw);
    if (h)
        *h = wxMax(sh, bh);
#endif
}


wxIndividualLayoutConstraint::wxIndividualLayoutConstraint()
{
    myEdge = wxTop;
    relationship = wxUnconstrained;
    margin = 0;
    value = 0;
    percent = 0;
    otherEdge = wxTop;
    done = false;
    otherWin = NULL;
}

void wxIndividualLayoutConstraint::Set(wxRelationship rel, wxWindowBase *otherW, wxEdge otherE, int val, int marg)
{
    if (rel == wxSameAs)
    {
                        SameAs(otherW, otherE, marg);
        return;
    }

    relationship = rel;
    otherWin = otherW;
    otherEdge = otherE;

    if ( rel == wxPercentOf )
    {
        percent = val;
    }
    else
    {
        value = val;
    }

    margin = marg;
}

void wxIndividualLayoutConstraint::LeftOf(wxWindowBase *sibling, int marg)
{
    Set(wxLeftOf, sibling, wxLeft, 0, marg);
}

void wxIndividualLayoutConstraint::RightOf(wxWindowBase *sibling, int marg)
{
    Set(wxRightOf, sibling, wxRight, 0, marg);
}

void wxIndividualLayoutConstraint::Above(wxWindowBase *sibling, int marg)
{
    Set(wxAbove, sibling, wxTop, 0, marg);
}

void wxIndividualLayoutConstraint::Below(wxWindowBase *sibling, int marg)
{
    Set(wxBelow, sibling, wxBottom, 0, marg);
}

void wxIndividualLayoutConstraint::SameAs(wxWindowBase *otherW, wxEdge edge, int marg)
{
    Set(wxPercentOf, otherW, edge, 100, marg);
}

void wxIndividualLayoutConstraint::PercentOf(wxWindowBase *otherW, wxEdge wh, int per)
{
    Set(wxPercentOf, otherW, wh, per);
}

void wxIndividualLayoutConstraint::Absolute(int val)
{
    value = val;
    relationship = wxAbsolute;
}

bool wxIndividualLayoutConstraint::ResetIfWin(wxWindowBase *otherW)
{
    if (otherW == otherWin)
    {
        myEdge = wxTop;
        relationship = wxAsIs;
        margin = 0;
        value = 0;
        percent = 0;
        otherEdge = wxTop;
        otherWin = NULL;
        return true;
    }

    return false;
}

bool wxIndividualLayoutConstraint::SatisfyConstraint(wxLayoutConstraints *constraints, wxWindowBase *win)
{
    if (relationship == wxAbsolute)
    {
        done = true;
        return true;
    }

    switch (myEdge)
    {
        case wxLeft:
        {
            switch (relationship)
            {
                case wxLeftOf:
                {
                                                                                int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxRightOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->right.GetDone() && constraints->width.GetDone())
                    {
                        value = (constraints->right.GetValue() - constraints->width.GetValue() + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->centreX.GetValue() - (constraints->width.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    int y;
                    win->GetPosition(&value, &y);
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxRight:
        {
            switch (relationship)
            {
                case wxLeftOf:
                {
                                                                                int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxRightOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01) - margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->left.GetDone() && constraints->width.GetDone())
                    {
                        value = (constraints->left.GetValue() + constraints->width.GetValue() - margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->centreX.GetValue() + (constraints->width.GetValue()/2) - margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    int x, y;
                    int w, h;
                    wxGetAsIs(win, &w, &h);
                    win->GetPosition(&x, &y);
                    value = x + w;
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxTop:
        {
            switch (relationship)
            {
                case wxAbove:
                {
                                                                                int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxBelow:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->bottom.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->bottom.GetValue() - constraints->height.GetValue() + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreY.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->centreY.GetValue() - (constraints->height.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    int x;
                    win->GetPosition(&x, &value);
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxBottom:
        {
            switch (relationship)
            {
                case wxAbove:
                {
                                                                                int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxBelow:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01) - margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->top.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->top.GetValue() + constraints->height.GetValue() - margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->centreY.GetDone() && constraints->height.GetDone())
                    {
                        value = (constraints->centreY.GetValue() + (constraints->height.GetValue()/2) - margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    int x, y;
                    int w, h;
                    wxGetAsIs(win, &w, &h);
                    win->GetPosition(&x, &y);
                    value = h + y;
                    done = true;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case wxCentreX:
        {
            switch (relationship)
            {
                case wxLeftOf:
                {
                                                                                int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxRightOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->left.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->left.GetValue() + (constraints->width.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->right.GetDone() && constraints->width.GetDone())
                    {
                        value = (int)(constraints->left.GetValue() - (constraints->width.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        case wxCentreY:
        {
            switch (relationship)
            {
                case wxAbove:
                {
                                                                                int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos - margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxBelow:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = edgePos + margin;
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->bottom.GetDone() && constraints->height.GetDone())
                    {
                        value = (int)(constraints->bottom.GetValue() - (constraints->height.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else if (constraints->top.GetDone() && constraints->height.GetDone())
                    {
                        value = (int)(constraints->top.GetValue() + (constraints->height.GetValue()/2) + margin);
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        case wxWidth:
        {
            switch (relationship)
            {
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01));
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    if (win)
                    {
                        int h;
                        wxGetAsIs(win, &value, &h);
                        done = true;
                        return true;
                    }
                    else return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->left.GetDone() && constraints->right.GetDone())
                    {
                        value = constraints->right.GetValue() - constraints->left.GetValue();
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->left.GetDone())
                    {
                        value = (int)(2*(constraints->centreX.GetValue() - constraints->left.GetValue()));
                        done = true;
                        return true;
                    }
                    else if (constraints->centreX.GetDone() && constraints->right.GetDone())
                    {
                        value = (int)(2*(constraints->right.GetValue() - constraints->centreX.GetValue()));
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        case wxHeight:
        {
            switch (relationship)
            {
                case wxPercentOf:
                {
                    int edgePos = GetEdge(otherEdge, win, otherWin);
                    if (edgePos != -1)
                    {
                        value = (int)(edgePos*(((float)percent)*0.01));
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                case wxAsIs:
                {
                    if (win)
                    {
                        int w;
                        wxGetAsIs(win, &w, &value);
                        done = true;
                        return true;
                    }
                    else return false;
                }
                case wxUnconstrained:
                {
                                                                                if (constraints->top.GetDone() && constraints->bottom.GetDone())
                    {
                        value = constraints->bottom.GetValue() - constraints->top.GetValue();
                        done = true;
                        return true;
                    }
                    else if (constraints->top.GetDone() && constraints->centreY.GetDone())
                    {
                        value = (int)(2*(constraints->centreY.GetValue() - constraints->top.GetValue()));
                        done = true;
                        return true;
                    }
                    else if (constraints->bottom.GetDone() && constraints->centreY.GetDone())
                    {
                        value = (int)(2*(constraints->bottom.GetValue() - constraints->centreY.GetValue()));
                        done = true;
                        return true;
                    }
                    else
                        return false;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

int wxIndividualLayoutConstraint::GetEdge(wxEdge which,
                                          wxWindowBase *thisWin,
                                          wxWindowBase *other) const
{
                if (other->GetChildren().Find((wxWindow*)thisWin))
    {
        switch (which)
        {
            case wxLeft:
                {
                    return 0;
                }
            case wxTop:
                {
                    return 0;
                }
            case wxRight:
                {
                    int w, h;
                    other->GetClientSizeConstraint(&w, &h);
                    return w;
                }
            case wxBottom:
                {
                    int w, h;
                    other->GetClientSizeConstraint(&w, &h);
                    return h;
                }
            case wxWidth:
                {
                    int w, h;
                    other->GetClientSizeConstraint(&w, &h);
                    return w;
                }
            case wxHeight:
                {
                    int w, h;
                    other->GetClientSizeConstraint(&w, &h);
                    return h;
                }
            case wxCentreX:
            case wxCentreY:
                {
                    int w, h;
                    other->GetClientSizeConstraint(&w, &h);
                    if (which == wxCentreX)
                        return (int)(w/2);
                    else
                        return (int)(h/2);
                }
            default:
                return -1;
        }
    }
    switch (which)
    {
        case wxLeft:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->left.GetDone())
                        return constr->left.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x, y;
                    other->GetPosition(&x, &y);
                    return x;
                }
            }
        case wxTop:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->top.GetDone())
                        return constr->top.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x, y;
                    other->GetPosition(&x, &y);
                    return y;
                }
            }
        case wxRight:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->right.GetDone())
                        return constr->right.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x, y, w, h;
                    other->GetPosition(&x, &y);
                    other->GetSize(&w, &h);
                    return (int)(x + w);
                }
            }
        case wxBottom:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->bottom.GetDone())
                        return constr->bottom.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x, y, w, h;
                    other->GetPosition(&x, &y);
                    other->GetSize(&w, &h);
                    return (int)(y + h);
                }
            }
        case wxWidth:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->width.GetDone())
                        return constr->width.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int w, h;
                    other->GetSize(&w, &h);
                    return w;
                }
            }
        case wxHeight:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->height.GetDone())
                        return constr->height.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int w, h;
                    other->GetSize(&w, &h);
                    return h;
                }
            }
        case wxCentreX:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->centreX.GetDone())
                        return constr->centreX.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x, y, w, h;
                    other->GetPosition(&x, &y);
                    other->GetSize(&w, &h);
                    return (int)(x + (w/2));
                }
            }
        case wxCentreY:
            {
                wxLayoutConstraints *constr = other->GetConstraints();
                                                if (constr)
                {
                    if (constr->centreY.GetDone())
                        return constr->centreY.GetValue();
                    else
                        return -1;
                }
                else
                {
                    int x, y, w, h;
                    other->GetPosition(&x, &y);
                    other->GetSize(&w, &h);
                    return (int)(y + (h/2));
                }
            }
        default:
            break;
    }
    return -1;
}

wxLayoutConstraints::wxLayoutConstraints()
{
    left.SetEdge(wxLeft);
    top.SetEdge(wxTop);
    right.SetEdge(wxRight);
    bottom.SetEdge(wxBottom);
    centreX.SetEdge(wxCentreX);
    centreY.SetEdge(wxCentreY);
    width.SetEdge(wxWidth);
    height.SetEdge(wxHeight);
}

bool wxLayoutConstraints::SatisfyConstraints(wxWindowBase *win, int *nChanges)
{
    int noChanges = 0;

    bool done = width.GetDone();
    bool newDone = (done ? true : width.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = height.GetDone();
    newDone = (done ? true : height.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = left.GetDone();
    newDone = (done ? true : left.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = top.GetDone();
    newDone = (done ? true : top.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = right.GetDone();
    newDone = (done ? true : right.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = bottom.GetDone();
    newDone = (done ? true : bottom.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = centreX.GetDone();
    newDone = (done ? true : centreX.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    done = centreY.GetDone();
    newDone = (done ? true : centreY.SatisfyConstraint(this, win));
    if (newDone != done)
        noChanges ++;

    *nChanges = noChanges;

    return AreSatisfied();
}

#endif 