




#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_IMAGE

#include "wx/quantize.h"

#ifndef WX_PRECOMP
    #include "wx/palette.h"
    #include "wx/image.h"
#endif

#ifdef __WXMSW__
    #include "wx/msw/private.h"
#endif

#include <stdlib.h>
#include <string.h>

#define RGB_RED       0
#define RGB_GREEN     1
#define RGB_BLUE      2

#define MAXJSAMPLE        255
#define CENTERJSAMPLE     128
#define BITS_IN_JSAMPLE   8
#define GETJSAMPLE(value) ((int) (value))

#define RIGHT_SHIFT(x,shft) ((x) >> (shft))

typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef unsigned int JDIMENSION;

typedef struct {
        void *cquantize;
        JDIMENSION output_width;
        JSAMPARRAY colormap;
        int actual_number_of_colors;
        int desired_number_of_colors;
        JSAMPLE *sample_range_limit, *srl_orig;
} j_decompress;

#if defined(__WINDOWS__)
    #define JMETHOD(type,methodname,arglist)  type (__cdecl methodname) arglist
#else
    #define JMETHOD(type,methodname,arglist)  type (methodname) arglist
#endif

typedef j_decompress *j_decompress_ptr;
struct jpeg_color_quantizer {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo, bool is_pre_scan));
  JMETHOD(void, color_quantize, (j_decompress_ptr cinfo,
                 JSAMPARRAY input_buf, JSAMPARRAY output_buf,
                 int num_rows));
  JMETHOD(void, finish_pass, (j_decompress_ptr cinfo));
  JMETHOD(void, new_color_map, (j_decompress_ptr cinfo));
};






#define R_SCALE 2       
#define G_SCALE 3       
#define B_SCALE 1       



#if RGB_RED == 0
#define C0_SCALE R_SCALE
#endif
#if RGB_BLUE == 0
#define C0_SCALE B_SCALE
#endif
#if RGB_GREEN == 1
#define C1_SCALE G_SCALE
#endif
#if RGB_RED == 2
#define C2_SCALE R_SCALE
#endif
#if RGB_BLUE == 2
#define C2_SCALE B_SCALE
#endif



#define MAXNUMCOLORS  (MAXJSAMPLE+1) 


#define HIST_C0_BITS  5     
#define HIST_C1_BITS  6     
#define HIST_C2_BITS  5     


#define HIST_C0_ELEMS  (1<<HIST_C0_BITS)
#define HIST_C1_ELEMS  (1<<HIST_C1_BITS)
#define HIST_C2_ELEMS  (1<<HIST_C2_BITS)


#define C0_SHIFT  (BITS_IN_JSAMPLE-HIST_C0_BITS)
#define C1_SHIFT  (BITS_IN_JSAMPLE-HIST_C1_BITS)
#define C2_SHIFT  (BITS_IN_JSAMPLE-HIST_C2_BITS)


typedef wxUint16 histcell;    

typedef histcell  * histptr;    

typedef histcell hist1d[HIST_C2_ELEMS]; 
typedef hist1d  * hist2d;   
typedef hist2d * hist3d;    




#if BITS_IN_JSAMPLE == 8
typedef wxInt16 FSERROR;      
typedef int LOCFSERROR;     
#else
typedef wxInt32 FSERROR;      
typedef wxInt32 LOCFSERROR;   
#endif

typedef FSERROR  *FSERRPTR; 




typedef struct {

   struct {
       void (*finish_pass)(j_decompress_ptr);
       void (*color_quantize)(j_decompress_ptr, JSAMPARRAY, JSAMPARRAY, int);
       void (*start_pass)(j_decompress_ptr, bool);
       void (*new_color_map)(j_decompress_ptr);
   } pub;

  
  JSAMPARRAY sv_colormap;   
  int desired;          

  
  hist3d histogram;     

  bool needs_zeroed;        

  
  FSERRPTR fserrors;        
  bool on_odd_row;      
  int * error_limiter;      
} my_cquantizer;

typedef my_cquantizer * my_cquantize_ptr;




void
prescan_quantize (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
          JSAMPARRAY WXUNUSED(output_buf), int num_rows)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  JSAMPROW ptr;
  histptr histp;
  hist3d histogram = cquantize->histogram;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    ptr = input_buf[row];
    for (col = width; col > 0; col--) {

      {

          
          histp = & histogram[GETJSAMPLE(ptr[0]) >> C0_SHIFT]
                 [GETJSAMPLE(ptr[1]) >> C1_SHIFT]
                 [GETJSAMPLE(ptr[2]) >> C2_SHIFT];
          
          if (++(*histp) <= 0)
            (*histp)--;
      }
      ptr += 3;
    }
  }
}




typedef struct {
  
  int c0min, c0max;
  int c1min, c1max;
  int c2min, c2max;
  
  wxInt32 volume;
  
  long colorcount;
} box;

typedef box * boxptr;


boxptr
find_biggest_color_pop (boxptr boxlist, int numboxes)


{
  boxptr boxp;
  int i;
  long maxc = 0;
  boxptr which = NULL;

  for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
    if (boxp->colorcount > maxc && boxp->volume > 0) {
      which = boxp;
      maxc = boxp->colorcount;
    }
  }
  return which;
}


boxptr
find_biggest_volume (boxptr boxlist, int numboxes)


{
  boxptr boxp;
  int i;
  wxInt32 maxv = 0;
  boxptr which = NULL;

  for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
    if (boxp->volume > maxv) {
      which = boxp;
      maxv = boxp->volume;
    }
  }
  return which;
}


void
update_box (j_decompress_ptr cinfo, boxptr boxp)


{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  histptr histp;
  int c0,c1,c2;
  int c0min,c0max,c1min,c1max,c2min,c2max;
  wxInt32 dist0,dist1,dist2;
  long ccount;

  c0min = boxp->c0min;  c0max = boxp->c0max;
  c1min = boxp->c1min;  c1max = boxp->c1max;
  c2min = boxp->c2min;  c2max = boxp->c2max;

  if (c0max > c0min)
    for (c0 = c0min; c0 <= c0max; c0++)
      for (c1 = c1min; c1 <= c1max; c1++) {
    histp = & histogram[c0][c1][c2min];
    for (c2 = c2min; c2 <= c2max; c2++)
      if (*histp++ != 0) {
        boxp->c0min = c0min = c0;
        goto have_c0min;
      }
      }
 have_c0min:
  if (c0max > c0min)
    for (c0 = c0max; c0 >= c0min; c0--)
      for (c1 = c1min; c1 <= c1max; c1++) {
    histp = & histogram[c0][c1][c2min];
    for (c2 = c2min; c2 <= c2max; c2++)
      if (*histp++ != 0) {
        boxp->c0max = c0max = c0;
        goto have_c0max;
      }
      }
 have_c0max:
  if (c1max > c1min)
    for (c1 = c1min; c1 <= c1max; c1++)
      for (c0 = c0min; c0 <= c0max; c0++) {
    histp = & histogram[c0][c1][c2min];
    for (c2 = c2min; c2 <= c2max; c2++)
      if (*histp++ != 0) {
        boxp->c1min = c1min = c1;
        goto have_c1min;
      }
      }
 have_c1min:
  if (c1max > c1min)
    for (c1 = c1max; c1 >= c1min; c1--)
      for (c0 = c0min; c0 <= c0max; c0++) {
    histp = & histogram[c0][c1][c2min];
    for (c2 = c2min; c2 <= c2max; c2++)
      if (*histp++ != 0) {
        boxp->c1max = c1max = c1;
        goto have_c1max;
      }
      }
 have_c1max:
  if (c2max > c2min)
    for (c2 = c2min; c2 <= c2max; c2++)
      for (c0 = c0min; c0 <= c0max; c0++) {
    histp = & histogram[c0][c1min][c2];
    for (c1 = c1min; c1 <= c1max; c1++, histp += HIST_C2_ELEMS)
      if (*histp != 0) {
        boxp->c2min = c2min = c2;
        goto have_c2min;
      }
      }
 have_c2min:
  if (c2max > c2min)
    for (c2 = c2max; c2 >= c2min; c2--)
      for (c0 = c0min; c0 <= c0max; c0++) {
    histp = & histogram[c0][c1min][c2];
    for (c1 = c1min; c1 <= c1max; c1++, histp += HIST_C2_ELEMS)
      if (*histp != 0) {
        boxp->c2max = c2max = c2;
        goto have_c2max;
      }
      }
 have_c2max:

  
  dist0 = ((c0max - c0min) << C0_SHIFT) * C0_SCALE;
  dist1 = ((c1max - c1min) << C1_SHIFT) * C1_SCALE;
  dist2 = ((c2max - c2min) << C2_SHIFT) * C2_SCALE;
  boxp->volume = dist0*dist0 + dist1*dist1 + dist2*dist2;

  
  ccount = 0;
  for (c0 = c0min; c0 <= c0max; c0++)
    for (c1 = c1min; c1 <= c1max; c1++) {
      histp = & histogram[c0][c1][c2min];
      for (c2 = c2min; c2 <= c2max; c2++, histp++)
    if (*histp != 0) {
      ccount++;
    }
    }
  boxp->colorcount = ccount;
}


int
median_cut (j_decompress_ptr cinfo, boxptr boxlist, int numboxes,
        int desired_colors)

{
  int n,lb;
  int c0,c1,c2,cmax;
  boxptr b1,b2;

  while (numboxes < desired_colors) {
    
    if ((numboxes*2) <= desired_colors) {
      b1 = find_biggest_color_pop(boxlist, numboxes);
    } else {
      b1 = find_biggest_volume(boxlist, numboxes);
    }
    if (b1 == NULL)     
      break;
    b2 = &boxlist[numboxes];    
    
    b2->c0max = b1->c0max; b2->c1max = b1->c1max; b2->c2max = b1->c2max;
    b2->c0min = b1->c0min; b2->c1min = b1->c1min; b2->c2min = b1->c2min;
    
    c0 = ((b1->c0max - b1->c0min) << C0_SHIFT) * C0_SCALE;
    c1 = ((b1->c1max - b1->c1min) << C1_SHIFT) * C1_SCALE;
    c2 = ((b1->c2max - b1->c2min) << C2_SHIFT) * C2_SCALE;
    
#if RGB_RED == 0
    cmax = c1; n = 1;
    if (c0 > cmax) { cmax = c0; n = 0; }
    if (c2 > cmax) { n = 2; }
#else
    cmax = c1; n = 1;
    if (c2 > cmax) { cmax = c2; n = 2; }
    if (c0 > cmax) { n = 0; }
#endif

    
    switch (n) {
    case 0:
      lb = (b1->c0max + b1->c0min) / 2;
      b1->c0max = lb;
      b2->c0min = lb+1;
      break;
    case 1:
      lb = (b1->c1max + b1->c1min) / 2;
      b1->c1max = lb;
      b2->c1min = lb+1;
      break;
    case 2:
      lb = (b1->c2max + b1->c2min) / 2;
      b1->c2max = lb;
      b2->c2min = lb+1;
      break;
    }
    
    update_box(cinfo, b1);
    update_box(cinfo, b2);
    numboxes++;
  }
  return numboxes;
}


void
compute_color (j_decompress_ptr cinfo, boxptr boxp, int icolor)

{
  
  
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  histptr histp;
  int c0,c1,c2;
  int c0min,c0max,c1min,c1max,c2min,c2max;
  long count;
  long total = 0;
  long c0total = 0;
  long c1total = 0;
  long c2total = 0;

  c0min = boxp->c0min;  c0max = boxp->c0max;
  c1min = boxp->c1min;  c1max = boxp->c1max;
  c2min = boxp->c2min;  c2max = boxp->c2max;

  for (c0 = c0min; c0 <= c0max; c0++)
    for (c1 = c1min; c1 <= c1max; c1++) {
      histp = & histogram[c0][c1][c2min];
      for (c2 = c2min; c2 <= c2max; c2++) {
    if ((count = *histp++) != 0) {
      total += count;
      c0total += ((c0 << C0_SHIFT) + ((1<<C0_SHIFT)>>1)) * count;
      c1total += ((c1 << C1_SHIFT) + ((1<<C1_SHIFT)>>1)) * count;
      c2total += ((c2 << C2_SHIFT) + ((1<<C2_SHIFT)>>1)) * count;
    }
      }
    }

  cinfo->colormap[0][icolor] = (JSAMPLE) ((c0total + (total>>1)) / total);
  cinfo->colormap[1][icolor] = (JSAMPLE) ((c1total + (total>>1)) / total);
  cinfo->colormap[2][icolor] = (JSAMPLE) ((c2total + (total>>1)) / total);
}


static void
select_colors (j_decompress_ptr cinfo, int desired_colors)

{
  boxptr boxlist;
  int numboxes;
  int i;

  
  boxlist = (boxptr) malloc(desired_colors * sizeof(box));
  
  numboxes = 1;
  boxlist[0].c0min = 0;
  boxlist[0].c0max = MAXJSAMPLE >> C0_SHIFT;
  boxlist[0].c1min = 0;
  boxlist[0].c1max = MAXJSAMPLE >> C1_SHIFT;
  boxlist[0].c2min = 0;
  boxlist[0].c2max = MAXJSAMPLE >> C2_SHIFT;
  
  update_box(cinfo, & boxlist[0]);
  
  numboxes = median_cut(cinfo, boxlist, numboxes, desired_colors);
  
  for (i = 0; i < numboxes; i++)
    compute_color(cinfo, & boxlist[i], i);
  cinfo->actual_number_of_colors = numboxes;

  free(boxlist); }






#define BOX_C0_LOG  (HIST_C0_BITS-3)
#define BOX_C1_LOG  (HIST_C1_BITS-3)
#define BOX_C2_LOG  (HIST_C2_BITS-3)

#define BOX_C0_ELEMS  (1<<BOX_C0_LOG) 
#define BOX_C1_ELEMS  (1<<BOX_C1_LOG)
#define BOX_C2_ELEMS  (1<<BOX_C2_LOG)

#define BOX_C0_SHIFT  (C0_SHIFT + BOX_C0_LOG)
#define BOX_C1_SHIFT  (C1_SHIFT + BOX_C1_LOG)
#define BOX_C2_SHIFT  (C2_SHIFT + BOX_C2_LOG)




static int
find_nearby_colors (j_decompress_ptr cinfo, int minc0, int minc1, int minc2,
            JSAMPLE colorlist[])

{
  int numcolors = cinfo->actual_number_of_colors;
  int maxc0, maxc1, maxc2;
  int centerc0, centerc1, centerc2;
  int i, x, ncolors;
  wxInt32 minmaxdist, min_dist, max_dist, tdist;
  wxInt32 mindist[MAXNUMCOLORS];  

  
  maxc0 = minc0 + ((1 << BOX_C0_SHIFT) - (1 << C0_SHIFT));
  centerc0 = (minc0 + maxc0) >> 1;
  maxc1 = minc1 + ((1 << BOX_C1_SHIFT) - (1 << C1_SHIFT));
  centerc1 = (minc1 + maxc1) >> 1;
  maxc2 = minc2 + ((1 << BOX_C2_SHIFT) - (1 << C2_SHIFT));
  centerc2 = (minc2 + maxc2) >> 1;

  
  minmaxdist = 0x7FFFFFFFL;

  for (i = 0; i < numcolors; i++) {
    
    x = GETJSAMPLE(cinfo->colormap[0][i]);
    if (x < minc0) {
      tdist = (x - minc0) * C0_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - maxc0) * C0_SCALE;
      max_dist = tdist*tdist;
    } else if (x > maxc0) {
      tdist = (x - maxc0) * C0_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - minc0) * C0_SCALE;
      max_dist = tdist*tdist;
    } else {
      
      min_dist = 0;
      if (x <= centerc0) {
    tdist = (x - maxc0) * C0_SCALE;
    max_dist = tdist*tdist;
      } else {
    tdist = (x - minc0) * C0_SCALE;
    max_dist = tdist*tdist;
      }
    }

    x = GETJSAMPLE(cinfo->colormap[1][i]);
    if (x < minc1) {
      tdist = (x - minc1) * C1_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxc1) * C1_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxc1) {
      tdist = (x - maxc1) * C1_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minc1) * C1_SCALE;
      max_dist += tdist*tdist;
    } else {
      
      if (x <= centerc1) {
    tdist = (x - maxc1) * C1_SCALE;
    max_dist += tdist*tdist;
      } else {
    tdist = (x - minc1) * C1_SCALE;
    max_dist += tdist*tdist;
      }
    }

    x = GETJSAMPLE(cinfo->colormap[2][i]);
    if (x < minc2) {
      tdist = (x - minc2) * C2_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxc2) * C2_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxc2) {
      tdist = (x - maxc2) * C2_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minc2) * C2_SCALE;
      max_dist += tdist*tdist;
    } else {
      
      if (x <= centerc2) {
    tdist = (x - maxc2) * C2_SCALE;
    max_dist += tdist*tdist;
      } else {
    tdist = (x - minc2) * C2_SCALE;
    max_dist += tdist*tdist;
      }
    }

    mindist[i] = min_dist;  
    if (max_dist < minmaxdist)
      minmaxdist = max_dist;
  }

  
  ncolors = 0;
  for (i = 0; i < numcolors; i++) {
    if (mindist[i] <= minmaxdist)
      colorlist[ncolors++] = (JSAMPLE) i;
  }
  return ncolors;
}


static void
find_best_colors (j_decompress_ptr cinfo, int minc0, int minc1, int minc2,
          int numcolors, JSAMPLE colorlist[], JSAMPLE bestcolor[])

{
  int ic0, ic1, ic2;
  int i, icolor;
  wxInt32 * bptr;    
  JSAMPLE * cptr;       
  wxInt32 dist0, dist1;       
  wxInt32 dist2;     
  wxInt32 xx0, xx1;       
  wxInt32 xx2;
  wxInt32 inc0, inc1, inc2;   
  
  wxInt32 bestdist[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];

  
  bptr = bestdist;
  for (i = BOX_C0_ELEMS*BOX_C1_ELEMS*BOX_C2_ELEMS-1; i >= 0; i--)
    *bptr++ = 0x7FFFFFFFL;

  

  
#define STEP_C0  ((1 << C0_SHIFT) * C0_SCALE)
#define STEP_C1  ((1 << C1_SHIFT) * C1_SCALE)
#define STEP_C2  ((1 << C2_SHIFT) * C2_SCALE)

  for (i = 0; i < numcolors; i++) {
    icolor = GETJSAMPLE(colorlist[i]);
    
    inc0 = (minc0 - GETJSAMPLE(cinfo->colormap[0][icolor])) * C0_SCALE;
    dist0 = inc0*inc0;
    inc1 = (minc1 - GETJSAMPLE(cinfo->colormap[1][icolor])) * C1_SCALE;
    dist0 += inc1*inc1;
    inc2 = (minc2 - GETJSAMPLE(cinfo->colormap[2][icolor])) * C2_SCALE;
    dist0 += inc2*inc2;
    
    inc0 = inc0 * (2 * STEP_C0) + STEP_C0 * STEP_C0;
    inc1 = inc1 * (2 * STEP_C1) + STEP_C1 * STEP_C1;
    inc2 = inc2 * (2 * STEP_C2) + STEP_C2 * STEP_C2;
    
    bptr = bestdist;
    cptr = bestcolor;
    xx0 = inc0;
    for (ic0 = BOX_C0_ELEMS-1; ic0 >= 0; ic0--) {
      dist1 = dist0;
      xx1 = inc1;
      for (ic1 = BOX_C1_ELEMS-1; ic1 >= 0; ic1--) {
    dist2 = dist1;
    xx2 = inc2;
    for (ic2 = BOX_C2_ELEMS-1; ic2 >= 0; ic2--) {
      if (dist2 < *bptr) {
        *bptr = dist2;
        *cptr = (JSAMPLE) icolor;
      }
      dist2 += xx2;
      xx2 += 2 * STEP_C2 * STEP_C2;
      bptr++;
      cptr++;
    }
    dist1 += xx1;
    xx1 += 2 * STEP_C1 * STEP_C1;
      }
      dist0 += xx0;
      xx0 += 2 * STEP_C0 * STEP_C0;
    }
  }
}


static void
fill_inverse_cmap (j_decompress_ptr cinfo, int c0, int c1, int c2)



{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  int minc0, minc1, minc2;  
  int ic0, ic1, ic2;
  JSAMPLE * cptr;  
  histptr cachep;  
  
  JSAMPLE colorlist[MAXNUMCOLORS];
  int numcolors;        
  
  JSAMPLE bestcolor[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];

  
  c0 >>= BOX_C0_LOG;
  c1 >>= BOX_C1_LOG;
  c2 >>= BOX_C2_LOG;

  
  minc0 = (c0 << BOX_C0_SHIFT) + ((1 << C0_SHIFT) >> 1);
  minc1 = (c1 << BOX_C1_SHIFT) + ((1 << C1_SHIFT) >> 1);
  minc2 = (c2 << BOX_C2_SHIFT) + ((1 << C2_SHIFT) >> 1);

  
  numcolors = find_nearby_colors(cinfo, minc0, minc1, minc2, colorlist);

  
  find_best_colors(cinfo, minc0, minc1, minc2, numcolors, colorlist,
           bestcolor);

  
  c0 <<= BOX_C0_LOG;        
  c1 <<= BOX_C1_LOG;
  c2 <<= BOX_C2_LOG;
  cptr = bestcolor;
  for (ic0 = 0; ic0 < BOX_C0_ELEMS; ic0++) {
    for (ic1 = 0; ic1 < BOX_C1_ELEMS; ic1++) {
      cachep = & histogram[c0+ic0][c1+ic1][c2];
      for (ic2 = 0; ic2 < BOX_C2_ELEMS; ic2++) {
    *cachep++ = (histcell) (GETJSAMPLE(*cptr++) + 1);
      }
    }
  }
}




void
pass2_no_dither (j_decompress_ptr cinfo,
         JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows)

{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  JSAMPROW inptr, outptr;
  histptr cachep;
  int c0, c1, c2;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    inptr = input_buf[row];
    outptr = output_buf[row];
    for (col = width; col > 0; col--) {
      
      c0 = GETJSAMPLE(*inptr++) >> C0_SHIFT;
      c1 = GETJSAMPLE(*inptr++) >> C1_SHIFT;
      c2 = GETJSAMPLE(*inptr++) >> C2_SHIFT;
      cachep = & histogram[c0][c1][c2];
      
      
      if (*cachep == 0)
    fill_inverse_cmap(cinfo, c0,c1,c2);
      
      *outptr++ = (JSAMPLE) (*cachep - 1);
    }
  }
}


void
pass2_fs_dither (j_decompress_ptr cinfo,
         JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows)

{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;
  LOCFSERROR cur0, cur1, cur2; 
  LOCFSERROR belowerr0, belowerr1, belowerr2; 
  LOCFSERROR bpreverr0, bpreverr1, bpreverr2; 
  FSERRPTR errorptr;   
  JSAMPROW inptr;       
  JSAMPROW outptr;      
  histptr cachep;
  int dir;          
  int dir3;         
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;
  JSAMPLE *range_limit = cinfo->sample_range_limit;
  int *error_limit = cquantize->error_limiter;
  JSAMPROW colormap0 = cinfo->colormap[0];
  JSAMPROW colormap1 = cinfo->colormap[1];
  JSAMPROW colormap2 = cinfo->colormap[2];


  for (row = 0; row < num_rows; row++) {
    inptr = input_buf[row];
    outptr = output_buf[row];
    if (cquantize->on_odd_row) {
      
      inptr += (width-1) * 3;   
      outptr += width-1;
      dir = -1;
      dir3 = -3;
      errorptr = cquantize->fserrors + (width+1)*3; 
      cquantize->on_odd_row = false; 
    } else {
      
      dir = 1;
      dir3 = 3;
      errorptr = cquantize->fserrors; 
      cquantize->on_odd_row = true; 
    }
    
    cur0 = cur1 = cur2 = 0;
    
    belowerr0 = belowerr1 = belowerr2 = 0;
    bpreverr0 = bpreverr1 = bpreverr2 = 0;

    for (col = width; col > 0; col--) {
      
      cur0 = RIGHT_SHIFT(cur0 + errorptr[dir3+0] + 8, 4);
      cur1 = RIGHT_SHIFT(cur1 + errorptr[dir3+1] + 8, 4);
      cur2 = RIGHT_SHIFT(cur2 + errorptr[dir3+2] + 8, 4);
      
      cur0 = error_limit[cur0];
      cur1 = error_limit[cur1];
      cur2 = error_limit[cur2];
      
      cur0 += GETJSAMPLE(inptr[0]);
      cur1 += GETJSAMPLE(inptr[1]);
      cur2 += GETJSAMPLE(inptr[2]);
      cur0 = GETJSAMPLE(range_limit[cur0]);
      cur1 = GETJSAMPLE(range_limit[cur1]);
      cur2 = GETJSAMPLE(range_limit[cur2]);
      
      cachep = & histogram[cur0>>C0_SHIFT][cur1>>C1_SHIFT][cur2>>C2_SHIFT];
      
      
      if (*cachep == 0)
    fill_inverse_cmap(cinfo, cur0>>C0_SHIFT,cur1>>C1_SHIFT,cur2>>C2_SHIFT);
      
      { int pixcode = *cachep - 1;
    *outptr = (JSAMPLE) pixcode;
    
    cur0 -= GETJSAMPLE(colormap0[pixcode]);
    cur1 -= GETJSAMPLE(colormap1[pixcode]);
    cur2 -= GETJSAMPLE(colormap2[pixcode]);
      }
      
      { LOCFSERROR bnexterr, delta;

    bnexterr = cur0;    
    delta = cur0 * 2;
    cur0 += delta;      
    errorptr[0] = (FSERROR) (bpreverr0 + cur0);
    cur0 += delta;      
    bpreverr0 = belowerr0 + cur0;
    belowerr0 = bnexterr;
    cur0 += delta;      
    bnexterr = cur1;    
    delta = cur1 * 2;
    cur1 += delta;      
    errorptr[1] = (FSERROR) (bpreverr1 + cur1);
    cur1 += delta;      
    bpreverr1 = belowerr1 + cur1;
    belowerr1 = bnexterr;
    cur1 += delta;      
    bnexterr = cur2;    
    delta = cur2 * 2;
    cur2 += delta;      
    errorptr[2] = (FSERROR) (bpreverr2 + cur2);
    cur2 += delta;      
    bpreverr2 = belowerr2 + cur2;
    belowerr2 = bnexterr;
    cur2 += delta;      
      }
      
      inptr += dir3;        
      outptr += dir;
      errorptr += dir3;     
    }
    
    errorptr[0] = (FSERROR) bpreverr0; 
    errorptr[1] = (FSERROR) bpreverr1;
    errorptr[2] = (FSERROR) bpreverr2;
  }
}




static void
init_error_limit (j_decompress_ptr cinfo)

{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  int * table;
  int in, out;

  table = (int *) malloc((MAXJSAMPLE*2+1) * sizeof(int));
  table += MAXJSAMPLE;      
  cquantize->error_limiter = table;

#define STEPSIZE ((MAXJSAMPLE+1)/16)
  
  out = 0;
  for (in = 0; in < STEPSIZE; in++, out++) {
    table[in] = out; table[-in] = -out;
  }
  
  for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1) {
    table[in] = out; table[-in] = -out;
  }
  
  for (; in <= MAXJSAMPLE; in++) {
    table[in] = out; table[-in] = -out;
  }
#undef STEPSIZE
}




void
finish_pass1 (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;

  
  cinfo->colormap = cquantize->sv_colormap;
  select_colors(cinfo, cquantize->desired);
  
  cquantize->needs_zeroed = true;
}


void
finish_pass2 (j_decompress_ptr WXUNUSED(cinfo))
{
  
}




void
start_pass_2_quant (j_decompress_ptr cinfo, bool is_pre_scan)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;
  hist3d histogram = cquantize->histogram;

  if (is_pre_scan) {
    
    cquantize->pub.color_quantize = prescan_quantize;
    cquantize->pub.finish_pass = finish_pass1;
    cquantize->needs_zeroed = true; 
  } else {
    
    cquantize->pub.color_quantize = pass2_fs_dither;
    cquantize->pub.finish_pass = finish_pass2;

    {
      size_t arraysize = (size_t) ((cinfo->output_width + 2) *
                   (3 * sizeof(FSERROR)));
      
      if (cquantize->fserrors == NULL)
          cquantize->fserrors = (wxInt16*) malloc(arraysize);
      
      memset((void  *) cquantize->fserrors, 0, arraysize);
      
      if (cquantize->error_limiter == NULL)
    init_error_limit(cinfo);
      cquantize->on_odd_row = false;
    }

  }
  
  if (cquantize->needs_zeroed) {
    for (int i = 0; i < HIST_C0_ELEMS; i++) {
      memset((void  *) histogram[i], 0,
        HIST_C1_ELEMS*HIST_C2_ELEMS * sizeof(histcell));
    }
    cquantize->needs_zeroed = false;
  }
}




void
new_color_map_2_quant (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr) cinfo->cquantize;

  
  cquantize->needs_zeroed = true;
}




void
jinit_2pass_quantizer (j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize;
  int i;

  cquantize = (my_cquantize_ptr) malloc(sizeof(my_cquantizer));
  cinfo->cquantize = (jpeg_color_quantizer *) cquantize;
  cquantize->pub.start_pass = start_pass_2_quant;
  cquantize->pub.new_color_map = new_color_map_2_quant;
  cquantize->fserrors = NULL;   
  cquantize->error_limiter = NULL;


  
  cquantize->histogram = (hist3d) malloc(HIST_C0_ELEMS * sizeof(hist2d));
  for (i = 0; i < HIST_C0_ELEMS; i++) {
    cquantize->histogram[i] = (hist2d) malloc(HIST_C1_ELEMS*HIST_C2_ELEMS * sizeof(histcell));
  }
  cquantize->needs_zeroed = true; 

  
  {
    
    int desired = cinfo->desired_number_of_colors;

    cquantize->sv_colormap = (JSAMPARRAY) malloc(sizeof(JSAMPROW) * 3);
    cquantize->sv_colormap[0] = (JSAMPROW) malloc(sizeof(JSAMPLE) * desired);
    cquantize->sv_colormap[1] = (JSAMPROW) malloc(sizeof(JSAMPLE) * desired);
    cquantize->sv_colormap[2] = (JSAMPROW) malloc(sizeof(JSAMPLE) * desired);

    cquantize->desired = desired;
  }

  
  {
    cquantize->fserrors = (FSERRPTR) malloc(
       (size_t) ((cinfo->output_width + 2) * (3 * sizeof(FSERROR))));
    
    init_error_limit(cinfo);
  }
}










void
prepare_range_limit_table (j_decompress_ptr cinfo)

{
  JSAMPLE * table;
  int i;

  table = (JSAMPLE *) malloc((5 * (MAXJSAMPLE+1) + CENTERJSAMPLE) * sizeof(JSAMPLE));
  cinfo->srl_orig = table;
  table += (MAXJSAMPLE+1);  
  cinfo->sample_range_limit = table;
  
  memset(table - (MAXJSAMPLE+1), 0, (MAXJSAMPLE+1) * sizeof(JSAMPLE));
  
  for (i = 0; i <= MAXJSAMPLE; i++)
    table[i] = (JSAMPLE) i;
  table += CENTERJSAMPLE;   
  
  for (i = CENTERJSAMPLE; i < 2*(MAXJSAMPLE+1); i++)
    table[i] = MAXJSAMPLE;
  
  memset(table + (2 * (MAXJSAMPLE+1)), 0,
      (2 * (MAXJSAMPLE+1) - CENTERJSAMPLE) * sizeof(JSAMPLE));
  memcpy(table + (4 * (MAXJSAMPLE+1) - CENTERJSAMPLE),
      cinfo->sample_range_limit, CENTERJSAMPLE * sizeof(JSAMPLE));
}






wxIMPLEMENT_DYNAMIC_CLASS(wxQuantize, wxObject);

void wxQuantize::DoQuantize(unsigned w, unsigned h, unsigned char **in_rows, unsigned char **out_rows,
    unsigned char *palette, int desiredNoColours)
{
    j_decompress dec;
    my_cquantize_ptr cquantize;

    dec.output_width = w;
    dec.desired_number_of_colors = desiredNoColours;
    prepare_range_limit_table(&dec);
    jinit_2pass_quantizer(&dec);
    cquantize = (my_cquantize_ptr) dec.cquantize;


    cquantize->pub.start_pass(&dec, true);
    cquantize->pub.color_quantize(&dec, in_rows, out_rows, h);
    cquantize->pub.finish_pass(&dec);

    cquantize->pub.start_pass(&dec, false);
    cquantize->pub.color_quantize(&dec, in_rows, out_rows, h);
    cquantize->pub.finish_pass(&dec);


    for (int i = 0; i < dec.desired_number_of_colors; i++) {
        palette[3 * i + 0] = dec.colormap[0][i];
        palette[3 * i + 1] = dec.colormap[1][i];
        palette[3 * i + 2] = dec.colormap[2][i];
    }

    for (int ii = 0; ii < HIST_C0_ELEMS; ii++) free(cquantize->histogram[ii]);
    free(cquantize->histogram);
    free(dec.colormap[0]);
    free(dec.colormap[1]);
    free(dec.colormap[2]);
    free(dec.colormap);
    free(dec.srl_orig);

        free((void*)(cquantize->error_limiter - MAXJSAMPLE)); 
    free(cquantize->fserrors);
    free(cquantize);
}


bool wxQuantize::Quantize(const wxImage& src, wxImage& dest,
                          wxPalette** pPalette,
                          int desiredNoColours,
                          unsigned char** eightBitData,
                          int flags)

{
    int i;

    int windowsSystemColourCount = 20;

    int paletteShift = 0;

            if (flags & wxQUANTIZE_INCLUDE_WINDOWS_COLOURS)
        paletteShift = windowsSystemColourCount;

    #ifdef __WXMSW__
    if ((flags & wxQUANTIZE_INCLUDE_WINDOWS_COLOURS) && (desiredNoColours > (256 - windowsSystemColourCount)))
        desiredNoColours = 256 - windowsSystemColourCount;
#endif

        int h = src.GetHeight();
    int w = src.GetWidth();
    unsigned char **rows = new unsigned char *[h];
    unsigned char *imgdt = src.GetData();
    for (i = 0; i < h; i++)
        rows[i] = imgdt + 3 * w * i;

    unsigned char palette[3*256];

        unsigned char *data8bit = new unsigned char[w * h];
    unsigned char **outrows = new unsigned char *[h];
    for (i = 0; i < h; i++)
        outrows[i] = data8bit + w * i;

        DoQuantize(w, h, rows, outrows, palette, desiredNoColours);

    delete[] rows;
    delete[] outrows;

    
    if (flags & wxQUANTIZE_FILL_DESTINATION_IMAGE)
    {
        if (!dest.IsOk())
            dest.Create(w, h);

        imgdt = dest.GetData();
        for (i = 0; i < w * h; i++)
        {
            unsigned char c = data8bit[i];
            imgdt[3 * i + 0] = palette[3 * c + 0];
            imgdt[3 * i + 1] = palette[3 * c + 1];
            imgdt[3 * i + 2] = palette[3 * c + 2];
        }
    }

    if (eightBitData && (flags & wxQUANTIZE_RETURN_8BIT_DATA))
    {
#ifdef __WXMSW__
        if (flags & wxQUANTIZE_INCLUDE_WINDOWS_COLOURS)
        {
                                    for (i = 0; i < w * h; i++)
                data8bit[i] = (unsigned char)(data8bit[i] + paletteShift);
        }
#endif
        *eightBitData = data8bit;
    }
    else
        delete[] data8bit;

#if wxUSE_PALETTE
        if (pPalette)
    {
        unsigned char* r = new unsigned char[256];
        unsigned char* g = new unsigned char[256];
        unsigned char* b = new unsigned char[256];

#ifdef __WXMSW__
                if (flags & wxQUANTIZE_INCLUDE_WINDOWS_COLOURS)
        {
            HDC hDC = ::GetDC(NULL);
            PALETTEENTRY* entries = new PALETTEENTRY[windowsSystemColourCount];
            ::GetSystemPaletteEntries(hDC, 0, windowsSystemColourCount, entries);
            ::ReleaseDC(NULL, hDC);

            for (i = 0; i < windowsSystemColourCount; i++)
            {
                r[i] = entries[i].peRed;
                g[i] = entries[i].peGreen;
                b[i] = entries[i].peBlue;
            }
            delete[] entries;
        }
#endif

        for (i = 0; i < desiredNoColours; i++)
        {
            r[i+paletteShift] = palette[i*3 + 0];
            g[i+paletteShift] = palette[i*3 + 1];
            b[i+paletteShift] = palette[i*3 + 2];
        }

                for (i = desiredNoColours+paletteShift; i < 256; i++)
        {
            r[i] = 0;
            g[i] = 0;
            b[i] = 0;
        }
        *pPalette = new wxPalette(256, r, g, b);
        delete[] r;
        delete[] g;
        delete[] b;
    }
#else     wxUnusedVar(pPalette);
#endif 
    return true;
}


bool wxQuantize::Quantize(const wxImage& src,
                          wxImage& dest,
                          int desiredNoColours,
                          unsigned char** eightBitData,
                          int flags)
{
    wxPalette* palette = NULL;
    if ( !Quantize(src, dest, & palette, desiredNoColours, eightBitData, flags) )
        return false;

#if wxUSE_PALETTE
    if (palette)
    {
        dest.SetPalette(* palette);
        delete palette;
    }
#endif 
    return true;
}

#endif
    