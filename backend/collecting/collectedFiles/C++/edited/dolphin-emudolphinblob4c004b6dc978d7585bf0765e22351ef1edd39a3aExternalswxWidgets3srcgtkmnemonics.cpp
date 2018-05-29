


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/log.h"
#include "wx/gtk/private/mnemonics.h"

namespace
{


const char *const entitiesNames[] =
{
    "&amp;", "&lt;", "&gt;", "&apos;", "&quot;"
};

} 


enum MnemonicsFlag
{
    MNEMONICS_REMOVE,
    MNEMONICS_CONVERT,
    MNEMONICS_CONVERT_MARKUP
};

static wxString GTKProcessMnemonics(const wxString& label, MnemonicsFlag flag)
{
    wxString labelGTK;
    labelGTK.reserve(label.length());
    for ( wxString::const_iterator i = label.begin(); i != label.end(); ++i )
    {
        wxChar ch = *i;

        switch ( ch )
        {
            case wxT('&'):
                if ( i + 1 == label.end() )
                {
                                        wxLogDebug(wxT("Invalid label \"%s\"."), label);
                    break;
                }

                if ( flag == MNEMONICS_CONVERT_MARKUP )
                {
                    bool isMnemonic = true;
                    size_t distanceFromEnd = label.end() - i;

                                        for (size_t j=0; j < WXSIZEOF(entitiesNames); j++)
                    {
                        const char *entity = entitiesNames[j];
                        size_t entityLen = wxStrlen(entity);

                        if (distanceFromEnd >= entityLen &&
                            wxString(i, i + entityLen) == entity)
                        {
                            labelGTK << entity;
                            i += entityLen - 1;                                                                                     isMnemonic = false;

                            break;
                        }
                    }

                    if (!isMnemonic)
                        continue;
                }

                ch = *(++i);                 switch ( ch )
                {
                    case wxT('&'):
                                                                        if ( flag == MNEMONICS_CONVERT_MARKUP )
                            labelGTK += wxT("&amp;");
                        else
                            labelGTK += wxT('&');
                        break;

                    case wxT('_'):
                        if ( flag != MNEMONICS_REMOVE )
                        {
                                                                                    labelGTK += wxT("_-");
                            break;
                        }
                        
                    default:
                        if ( flag != MNEMONICS_REMOVE )
                            labelGTK += wxT('_');
                        labelGTK += ch;
                }
                break;

            case wxT('_'):
                if ( flag != MNEMONICS_REMOVE )
                {
                                                            labelGTK += wxT("__");
                    break;
                }
                
            default:
                labelGTK += ch;
        }
    }

    return labelGTK;
}


wxString wxGTKRemoveMnemonics(const wxString& label)
{
    return GTKProcessMnemonics(label, MNEMONICS_REMOVE);
}

wxString wxConvertMnemonicsToGTK(const wxString& label)
{
    return GTKProcessMnemonics(label, MNEMONICS_CONVERT);
}

wxString wxConvertMnemonicsToGTKMarkup(const wxString& label)
{
    return GTKProcessMnemonics(label, MNEMONICS_CONVERT_MARKUP);
}

wxString wxConvertMnemonicsFromGTK(const wxString& gtkLabel)
{
    wxString label;
    for ( const wxChar *pc = gtkLabel.c_str(); *pc; pc++ )
    {
        
        if ( *pc == wxT('_') && *(pc+1) == wxT('_'))
        {
                        label += wxT('_');
            pc++;
        }
        else if ( *pc == wxT('_') )
        {
                        label += wxT('&');
        }
        else if ( *pc == wxT('&') )
        {
                        label += wxT("&&");
        }
        else
        {
                                    label += *pc;
        }
    }

    return label;
}

