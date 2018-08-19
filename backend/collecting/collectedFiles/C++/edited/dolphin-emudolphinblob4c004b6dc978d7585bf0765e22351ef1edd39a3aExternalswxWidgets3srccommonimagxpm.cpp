


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_XPM

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/utils.h"
#endif

#include "wx/imagxpm.h"
#include "wx/wfstream.h"
#include "wx/xpmdecod.h"
#include "wx/filename.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxXPMHandler,wxImageHandler);


#if wxUSE_STREAMS

bool wxXPMHandler::LoadFile(wxImage *image,
                            wxInputStream& stream,
                            bool WXUNUSED(verbose), int WXUNUSED(index))
{
    wxXPMDecoder decoder;

    wxImage img = decoder.ReadFile(stream);
    if ( !img.IsOk() )
        return false;
    *image = img;
    return true;
}

namespace
{

void
MakeValidCIdent(wxString* str)
{
    const wxChar chUnderscore = wxT('_');

    for ( wxString::iterator it = str->begin(); it != str->end(); ++it )
    {
        const wxChar ch = *it;
        if ( wxIsdigit(ch) )
        {
            if ( it == str->begin() )
            {
                                str->insert(0, chUnderscore);                 it = str->begin();                 continue;
            }
        }
        else if ( !wxIsalpha(ch) && ch != chUnderscore )
        {
                        *it = chUnderscore;
        }
    }

            str->Replace(wxT("__"), wxT("_"));
}

} 
bool wxXPMHandler::SaveFile(wxImage * image,
                            wxOutputStream& stream, bool WXUNUSED(verbose))
{
        #define MaxCixels  92
    static const char Cixel[MaxCixels+1] =
                         " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjk"
                         "lzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";
    int i, j, k;

    wxImageHistogram histogram;
    int cols = int(image->ComputeHistogram(histogram));

    int chars_per_pixel = 1;
    for ( k = MaxCixels; cols > k; k *= MaxCixels)
        chars_per_pixel++;

        wxString sName;
    if ( image->HasOption(wxIMAGE_OPTION_FILENAME) )
    {
        sName = wxFileName(image->GetOption(wxIMAGE_OPTION_FILENAME)).GetName();
        MakeValidCIdent(&sName);
        sName << wxT("_xpm");
    }

    if ( !sName.empty() )
        sName = wxString(wxT("\nstatic const char *")) + sName;
    else
        sName = wxT("\nstatic const char *xpm_data");
    stream.Write( (const char*) sName.ToAscii(), sName.Len() );

    char tmpbuf[200];
            sprintf(tmpbuf,
               "[] = {\n"
               "\n"
               "\"%i %i %i %i\",\n",
               image->GetWidth(), image->GetHeight(), cols, chars_per_pixel);
    stream.Write(tmpbuf, strlen(tmpbuf));

        char *symbols_data = new char[cols * (chars_per_pixel+1)];
    char **symbols = new char*[cols];

        unsigned long mask_key = 0x1000000 ;
    if (image->HasMask())
        mask_key = (image->GetMaskRed() << 16) |
                   (image->GetMaskGreen() << 8) | image->GetMaskBlue();

        for (wxImageHistogram::iterator entry = histogram.begin();
         entry != histogram.end(); ++entry )
    {
        unsigned long index = entry->second.index;
        symbols[index] = symbols_data + index * (chars_per_pixel+1);
        char *sym = symbols[index];

        for (j = 0; j < chars_per_pixel; j++)
        {
            sym[j] = Cixel[index % MaxCixels];
            index /= MaxCixels;
        }
        sym[j] = '\0';

        unsigned long key = entry->first;

        if (key == 0)
            sprintf( tmpbuf, "\"%s c Black\",\n", sym);
        else if (key == mask_key)
            sprintf( tmpbuf, "\"%s c None\",\n", sym);
        else
        {
            wxByte r = wxByte(key >> 16);
            wxByte g = wxByte(key >> 8);
            wxByte b = wxByte(key);
            sprintf(tmpbuf, "\"%s c #%02X%02X%02X\",\n", sym, r, g, b);
        }
        stream.Write( tmpbuf, strlen(tmpbuf) );
    }

    stream.Write("\n", 13);

    unsigned char *data = image->GetData();
    for (j = 0; j < image->GetHeight(); j++)
    {
        char tmp_c;
        tmp_c = '\"'; stream.Write(&tmp_c, 1);
        for (i = 0; i < image->GetWidth(); i++, data += 3)
        {
            unsigned long key = (data[0] << 16) | (data[1] << 8) | (data[2]);
            stream.Write(symbols[histogram[key].index], chars_per_pixel);
        }
        tmp_c = '\"'; stream.Write(&tmp_c, 1);
        if ( j + 1 < image->GetHeight() )
        {
            tmp_c = ','; stream.Write(&tmp_c, 1);
        }
        tmp_c = '\n'; stream.Write(&tmp_c, 1);
    }
    stream.Write("};\n", 3 );

        delete[] symbols;
    delete[] symbols_data;

    return true;
}

bool wxXPMHandler::DoCanRead(wxInputStream& stream)
{
    wxXPMDecoder decoder;
    return decoder.CanRead(stream);
         }

#endif  
#endif 