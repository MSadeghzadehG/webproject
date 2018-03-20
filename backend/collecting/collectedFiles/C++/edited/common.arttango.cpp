


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ARTPROVIDER_TANGO

#ifndef WX_PRECOMP
    #include "wx/image.h"
    #include "wx/log.h"
#endif 
#include "wx/artprov.h"

#include "wx/mstream.h"


#include "../../art/tango/application_x_executable.h"
#include "../../art/tango/dialog_error.h"
#include "../../art/tango/dialog_information.h"
#include "../../art/tango/dialog_warning.h"
#include "../../art/tango/document_new.h"
#include "../../art/tango/document_open.h"
#include "../../art/tango/document_print.h"
#include "../../art/tango/document_save.h"
#include "../../art/tango/document_save_as.h"
#include "../../art/tango/drive_harddisk.h"
#include "../../art/tango/drive_optical.h"
#include "../../art/tango/drive_removable_media.h"
#include "../../art/tango/edit_copy.h"
#include "../../art/tango/edit_cut.h"
#include "../../art/tango/edit_delete.h"
#include "../../art/tango/edit_find.h"
#include "../../art/tango/edit_find_replace.h"
#include "../../art/tango/edit_paste.h"
#include "../../art/tango/edit_redo.h"
#include "../../art/tango/edit_undo.h"
#include "../../art/tango/folder.h"
#include "../../art/tango/folder_new.h"
#include "../../art/tango/folder_open.h"
#include "../../art/tango/fullscreen.h"
#include "../../art/tango/go_down.h"
#include "../../art/tango/go_first.h"
#include "../../art/tango/go_home.h"
#include "../../art/tango/go_last.h"
#include "../../art/tango/go_next.h"
#include "../../art/tango/go_previous.h"
#include "../../art/tango/go_up.h"
#include "../../art/tango/image_missing.h"
#include "../../art/tango/text_x_generic.h"
#include "../../art/tango/list_add.h"
#include "../../art/tango/list_remove.h"


namespace
{

class wxTangoArtProvider : public wxArtProvider
{
public:
    wxTangoArtProvider()
    {
        m_imageHandledAdded = false;
    }

protected:
    virtual wxBitmap CreateBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size) wxOVERRIDE;

private:
    bool m_imageHandledAdded;

    wxDECLARE_NO_COPY_CLASS(wxTangoArtProvider);
};

} 

wxBitmap
wxTangoArtProvider::CreateBitmap(const wxArtID& id,
                                 const wxArtClient& client,
                                 const wxSize& sizeHint)
{
                    
                #define BITMAP_ARRAY_NAME(name, size) \
        name ## _ ## size ## x ## size ## _png
    #define BITMAP_DATA_FOR_SIZE(name, size) \
        BITMAP_ARRAY_NAME(name, size), sizeof(BITMAP_ARRAY_NAME(name, size))
    #define BITMAP_DATA(name) \
        BITMAP_DATA_FOR_SIZE(name, 16), BITMAP_DATA_FOR_SIZE(name, 24)

    static const struct BitmapEntry
    {
        const char *id;
        const unsigned char *data16;
        size_t len16;
        const unsigned char *data24;
        size_t len24;
    } s_allBitmaps[] =
    {
                                        
        { wxART_GO_BACK,            BITMAP_DATA(go_previous)                },
        { wxART_GO_FORWARD,         BITMAP_DATA(go_next)                    },
        { wxART_GO_UP,              BITMAP_DATA(go_up)                      },
        { wxART_GO_DOWN,            BITMAP_DATA(go_down)                    },
                { wxART_GO_HOME,            BITMAP_DATA(go_home)                    },
        { wxART_GOTO_FIRST,         BITMAP_DATA(go_first)                   },
        { wxART_GOTO_LAST,          BITMAP_DATA(go_last)                    },

        { wxART_FILE_OPEN,          BITMAP_DATA(document_open)              },
        { wxART_FILE_SAVE,          BITMAP_DATA(document_save)              },
        { wxART_FILE_SAVE_AS,       BITMAP_DATA(document_save_as)           },
        { wxART_PRINT,              BITMAP_DATA(document_print)             },

        
        { wxART_NEW_DIR,            BITMAP_DATA(folder_new)                 },
        { wxART_HARDDISK,           BITMAP_DATA(drive_harddisk)             },
                { wxART_FLOPPY,             BITMAP_DATA(drive_removable_media)      },
        { wxART_CDROM,              BITMAP_DATA(drive_optical)              },
        { wxART_REMOVABLE,          BITMAP_DATA(drive_removable_media)      },

        { wxART_FOLDER,             BITMAP_DATA(folder)                     },
        { wxART_FOLDER_OPEN,        BITMAP_DATA(folder_open)                },
        
        { wxART_EXECUTABLE_FILE,    BITMAP_DATA(application_x_executable)   },
        { wxART_NORMAL_FILE,        BITMAP_DATA(text_x_generic)             },

                                { wxART_ERROR,              BITMAP_DATA(dialog_error)               },
        { wxART_QUESTION,           BITMAP_DATA(dialog_information)         },
        { wxART_WARNING,            BITMAP_DATA(dialog_warning)             },
        { wxART_INFORMATION,        BITMAP_DATA(dialog_information)         },

        { wxART_MISSING_IMAGE,      BITMAP_DATA(image_missing)              },

        { wxART_COPY,               BITMAP_DATA(edit_copy)                  },
        { wxART_CUT,                BITMAP_DATA(edit_cut)                   },
        { wxART_PASTE,              BITMAP_DATA(edit_paste)                 },
        { wxART_DELETE,             BITMAP_DATA(edit_delete)                },
        { wxART_NEW,                BITMAP_DATA(document_new)               },
        { wxART_UNDO,               BITMAP_DATA(edit_undo)                  },
        { wxART_REDO,               BITMAP_DATA(edit_redo)                  },

        { wxART_PLUS,               BITMAP_DATA(list_add)                   },
        { wxART_MINUS,              BITMAP_DATA(list_remove)                },

                        
        { wxART_FIND,               BITMAP_DATA(edit_find)                  },
        { wxART_FIND_AND_REPLACE,   BITMAP_DATA(edit_find_replace)          },
        { wxART_FULL_SCREEN,        BITMAP_DATA(full_screen)                },
    };

    #undef BITMAP_ARRAY_NAME
    #undef BITMAP_DATA_FOR_SIZE
    #undef BITMAP_DATA

    for ( unsigned n = 0; n < WXSIZEOF(s_allBitmaps); n++ )
    {
        const BitmapEntry& entry = s_allBitmaps[n];
        if ( entry.id != id )
            continue;

                
        wxSize size;
        bool sizeIsAHint;
        if ( sizeHint == wxDefaultSize )
        {
                        size = GetNativeSizeHint(client);

            if ( size == wxDefaultSize )
            {
                                                                                if ( client == wxART_MENU || client == wxART_BUTTON )
                    size = wxSize(16, 16);
                else
                    size = wxSize(24, 24);
            }

                                    sizeIsAHint = false;
        }
        else         {
                        size = sizeHint;

                                    sizeIsAHint = true;
        }

        enum
        {
            TangoSize_16,
            TangoSize_24
        } tangoSize;

                        if ( size.x <= 16 && size.y <= 16 )
            tangoSize = TangoSize_16;
        else
            tangoSize = TangoSize_24;

        const unsigned char *data;
        size_t len;
        switch ( tangoSize )
        {
            default:
                wxFAIL_MSG( "Unsupported Tango bitmap size" );
                wxFALLTHROUGH;

            case TangoSize_16:
                data = entry.data16;
                len = entry.len16;
                break;

            case TangoSize_24:
                data = entry.data24;
                len = entry.len24;
                break;
        }

        wxMemoryInputStream is(data, len);

                                                if ( !m_imageHandledAdded )
        {
                                    if ( !wxImage::FindHandler(wxBITMAP_TYPE_PNG) )
                wxImage::AddHandler(new wxPNGHandler);

                        m_imageHandledAdded = true;
        }

        wxImage image(is, wxBITMAP_TYPE_PNG);
        if ( !image.IsOk() )
        {
                                    wxLogDebug("Failed to load embedded PNG image for \"%s\"", id);
            return wxNullBitmap;
        }

        if ( !sizeIsAHint )
        {
                        image.Rescale(size.x, size.y, wxIMAGE_QUALITY_HIGH);
        }

        return image;
    }

        return wxNullBitmap;
}


void wxArtProvider::InitTangoProvider()
{
    wxArtProvider::PushBack(new wxTangoArtProvider);
}

#endif 