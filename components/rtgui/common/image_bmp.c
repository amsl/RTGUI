#include <rtthread.h>
#include <rtgui/dc_hw.h>
#include <rtgui/image.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/image_bmp.h>
#include <rtgui/blit.h>

#ifdef _WIN32
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#endif

#ifdef RTGUI_IMAGE_BMP

static rt_bool_t rtgui_image_bmp_check(struct rtgui_filerw* file);
static rt_bool_t rtgui_image_bmp_load(struct rtgui_image* image, struct rtgui_filerw* file, rt_bool_t load);
static void rtgui_image_bmp_unload(struct rtgui_image* image);
static void rtgui_image_bmp_blit(struct rtgui_image* image, struct rtgui_dc* dc, struct rtgui_rect* rect);

struct rtgui_image_engine rtgui_image_bmp_engine =
{
	"bmp",
	{ RT_NULL },
	rtgui_image_bmp_check,
	rtgui_image_bmp_load,
	rtgui_image_bmp_unload,
	rtgui_image_bmp_blit
};

rt_inline rt_uint32_t rtgui_filerw_read32(struct rtgui_filerw* src)
{
	rt_uint32_t value;
	rtgui_filerw_read(src, &value, (sizeof value), 1);
	return value;
}

rt_inline rt_uint16_t rtgui_filerw_read16(struct rtgui_filerw* src)
{
	rt_uint16_t value;
	rtgui_filerw_read(src, &value, (sizeof value), 1);
	return value;

}

static rt_bool_t rtgui_image_bmp_check(struct rtgui_filerw* file)
{
	char magic[2];
	rt_bool_t is_bmp;
	rt_off_t start;

	if ( !file ) return 0;

	start = rtgui_filerw_tell(file);

	/* move to the beginning of file */
	rtgui_filerw_seek(file, 0, RTGUI_FILE_SEEK_SET);

	is_bmp = RT_FALSE;
	if ( rtgui_filerw_read(file, magic, 1, sizeof(magic)) == sizeof(magic) )
	{
		if (magic[0] == 'B' &&
				magic[1] == 'M')
		{
			is_bmp = RT_TRUE;
		}
	}

	rtgui_filerw_seek(file, start, RTGUI_FILE_SEEK_SET);

	return(is_bmp);
}

static struct rtgui_image_palette* rtgui_image_bmp_load_palette(struct rtgui_image_bmp_header* header, struct rtgui_filerw* src)
{
	/* Load the palette, if any */
	rt_size_t i;
	struct rtgui_image_palette* palette;

	if (header->biClrUsed == 0)
		header->biClrUsed = 1 << header->biBitCount;

	palette = rtgui_image_palette_create(header->biClrUsed);
	if (palette == RT_NULL) return RT_NULL;

	if ( header->biSize == 12 )
	{
		rt_uint8_t r, g, b;
		for ( i = 0; i < (int)header->biClrUsed; ++i )
		{
			rtgui_filerw_read(src, &b, 1, 1);
			rtgui_filerw_read(src, &g, 1, 1);
			rtgui_filerw_read(src, &r, 1, 1);
			palette->colors[i] = RTGUI_RGB(r, g, b);
		}
	}
	else
	{
		rt_uint8_t r, g, b, a;
		for ( i = 0; i < (int)header->biClrUsed; ++i )
		{
			rtgui_filerw_read(src, &b, 1, 1);
			rtgui_filerw_read(src, &g, 1, 1);
			rtgui_filerw_read(src, &r, 1, 1);
			rtgui_filerw_read(src, &a, 1, 1);
			palette->colors[i] = RTGUI_ARGB(a, r, g, b);
		}
	}

	return palette;
}

static rt_bool_t rtgui_image_bmp_load(struct rtgui_image* image, struct rtgui_filerw* src, rt_bool_t load)
{
	rt_uint32_t Rmask;
	rt_uint32_t Gmask;
	rt_uint32_t Bmask;
	int ExpandBMP, bmpPitch;
	struct rtgui_image_bmp* bmp;
	struct rtgui_image_bmp_header* header;
	struct rtgui_image_palette* palette;

	bmp = RT_NULL;
	header = RT_NULL;
	palette = RT_NULL;
	header = (struct rtgui_image_bmp_header*) rtgui_malloc(sizeof(struct rtgui_image_bmp_header));
	if (header == RT_NULL) return RT_FALSE;

	if ( rtgui_filerw_read(src, header->magic, 1, sizeof(header->magic)) ==
		sizeof(sizeof(header->magic)) )
	{
		if (header->magic[0] != 'B' ||
			header->magic[1] != 'M')
		{
			goto __exit;
		}
	}

	header->bfSize		= rtgui_filerw_read32(src);
	header->bfReserved1	= rtgui_filerw_read16(src);
	header->bfReserved2	= rtgui_filerw_read16(src);
	header->bfOffBits	= rtgui_filerw_read32(src);

	/* Read the Win32 BITMAPINFOHEADER */
	header->biSize		= rtgui_filerw_read32(src);
	if ( header->biSize == 12 )
	{
		header->biWidth		= (rt_uint32_t)rtgui_filerw_read16(src);
		header->biHeight	= (rt_uint32_t)rtgui_filerw_read16(src);
		header->biPlanes	= rtgui_filerw_read16(src);
		header->biBitCount	= rtgui_filerw_read16(src);
		header->biCompression	= BI_RGB;
		header->biSizeImage	= 0;
		header->biXPelsPerMeter	= 0;
		header->biYPelsPerMeter	= 0;
		header->biClrUsed	= 0;

		header->biClrImportant	= 0;

	}
	else
	{
		header->biWidth		= rtgui_filerw_read32(src);
		header->biHeight	= rtgui_filerw_read32(src);
		header->biPlanes	= rtgui_filerw_read16(src);
		header->biBitCount	= rtgui_filerw_read16(src);
		header->biCompression	= rtgui_filerw_read32(src);
		header->biSizeImage	= rtgui_filerw_read32(src);
		header->biXPelsPerMeter	= rtgui_filerw_read32(src);
		header->biYPelsPerMeter	= rtgui_filerw_read32(src);
		header->biClrUsed	= rtgui_filerw_read32(src);
		header->biClrImportant	= rtgui_filerw_read32(src);

	}

	/* allocate palette and expand 1 and 4 bit bitmaps to 8 bits per pixel */
	switch (header->biBitCount)
	{
	case 1:
		ExpandBMP = header->biBitCount;
		palette = rtgui_image_bmp_load_palette(header, src);
		header->biBitCount = 8;
		break;

	case 4:
		ExpandBMP = header->biBitCount;
		palette = rtgui_image_bmp_load_palette(header, src);
		header->biBitCount = 8;
		break;

	case 8:
		palette = rtgui_image_bmp_load_palette(header, src);
		ExpandBMP = 0;
		break;

	default:
		ExpandBMP = 0;
		break;
	}

	/* We don't support any BMP compression right now */
	Rmask = Gmask = Bmask = 0;
	switch (header->biCompression)
	{
	case BI_RGB:
		/* If there are no masks, use the defaults */
		if ( header->bfOffBits == (14 + header->biSize) )
		{
			/* Default values for the BMP format */
			switch (header->biBitCount)
			{
			case 15:
			case 16:
				Rmask = 0x7C00;
				Gmask = 0x03E0;
				Bmask = 0x001F;
				break;

			case 24:
			case 32:
				Rmask = 0x00FF0000;
				Gmask = 0x0000FF00;
				Bmask = 0x000000FF;
				break;

			default:
				break;
			}
			break;
		}
		/* Fall through -- read the RGB masks */

	case BI_BITFIELDS:
		switch (header->biBitCount)
		{
		case 15:
		case 16:
		case 32:
			Rmask = rtgui_filerw_read32(src);
			Gmask = rtgui_filerw_read32(src);
			Bmask = rtgui_filerw_read32(src);
			break;

		default:
			break;
		}
		break;

	default:
		rt_kprintf("Compressed BMP files not supported\n");
		goto __exit;
	}

	bmp = (struct rtgui_image_bmp*) rtgui_malloc(sizeof(struct rtgui_image_bmp));
	if (bmp == RT_NULL)
		goto __exit;

	/* set image information */
	image->w = header->biWidth;
	image->h = header->biHeight;
	image->engine = &rtgui_image_bmp_engine;
	image->data = bmp;
	bmp->filerw = src;
	bmp->byte_per_pixel = header->biBitCount/8;
	bmp->pitch = image->w * bmp->byte_per_pixel;
	bmp->pixel_offset = header->bfOffBits;
	bmp->Rmask = Rmask; bmp->Gmask = Gmask; bmp->Bmask = Bmask;
	bmp->ExpandBMP = ExpandBMP;
	if (palette != RT_NULL) image->palette = palette;

	/* get padding */
	switch (ExpandBMP)
	{
	case 1:
		bmpPitch = (header->biWidth + 7) >> 3;
		bmp->pad  = (((bmpPitch)%4) ? (4-((bmpPitch)%4)) : 0);
		break;

	case 4:
		bmpPitch = (header->biWidth + 1) >> 1;
		bmp->pad  = (((bmpPitch)%4) ? (4-((bmpPitch)%4)) : 0);
		break;

	default:
		bmp->pad  = ((bmp->pitch%4) ? (4-(bmp->pitch%4)) : 0);
		break;
	}

	if (load == RT_TRUE)
	{
		rt_uint8_t *bits;
		rt_uint32_t i;

		/* load all pixels */
		bmp->pixels = rtgui_malloc(image->h * bmp->pitch);
		if (bmp->pixels == RT_NULL)
			goto __exit;

		/* Read the pixels.  Note that the bmp image is upside down */
		if ( rtgui_filerw_seek(src, bmp->pixel_offset, RTGUI_FILE_SEEK_SET) < 0)
			goto __exit;

		bits = bmp->pixels + image->h * bmp->pitch;
		while ( bits > bmp->pixels )
		{
			bits -= bmp->pitch;
			switch (ExpandBMP)
			{
			case 1:
			case 4:
				{
					rt_uint8_t pixel = 0;
					int   shift = (8 - ExpandBMP);
					for ( i=0; i < image->w; ++i )
					{
						if ( i % (8/ExpandBMP) == 0 )
						{
							if ( !rtgui_filerw_read(src, &pixel, 1, 1) )
								goto __exit;

						}
						*(bits+i) = (pixel>>shift);
						pixel <<= ExpandBMP;
					}
				}
				break;

			default:
				if ( rtgui_filerw_read(src, bits, 1, bmp->pitch) != bmp->pitch ) goto __exit;
				break;
			}

			/* Skip padding bytes  */
			if ( bmp->pad )
			{
				rt_uint8_t padbyte;
				for ( i=0; i < bmp->pad; ++i )
				{
					rtgui_filerw_read(src, &padbyte, 1, 1);
				}
			}
		}

		rtgui_filerw_close(bmp->filerw);
		bmp->line_pixels = RT_NULL;
		bmp->filerw = RT_NULL;
		bmp->pixel_offset = 0;
	}
	else
	{
		bmp->pixels = RT_NULL;
		bmp->line_pixels = rtgui_malloc(bmp->pitch);
	}

	return RT_TRUE;

__exit:
	rtgui_free(header);
	if (palette != RT_NULL)
	{
		rtgui_free(palette);
		image->palette = RT_NULL;
	}

	if (bmp != RT_NULL)
		rtgui_free(bmp->pixels);
	rtgui_free(bmp);
	return RT_FALSE;
}

static void rtgui_image_bmp_unload(struct rtgui_image* image)
{
	struct rtgui_image_bmp* bmp;

	if (image != RT_NULL)
	{
		bmp = (struct rtgui_image_bmp*) image->data;

		if (bmp->pixels != RT_NULL) rtgui_free(bmp->pixels);
		if (bmp->line_pixels != RT_NULL) rtgui_free(bmp->line_pixels);
		if (bmp->filerw != RT_NULL)
		{
			rtgui_filerw_close(bmp->filerw);
			bmp->filerw = RT_NULL;
		}

		/* release data */
		rtgui_free(bmp);
	}
}

static void rtgui_image_bmp_blit(struct rtgui_image* image, struct rtgui_dc* dc, struct rtgui_rect* dst_rect)
{
	int y;
	rt_uint16_t w, h;
	struct rtgui_image_bmp* bmp;

	RT_ASSERT(image != RT_NULL || dc != RT_NULL || dst_rect != RT_NULL);

	/* this dc is not visible */
	if (rtgui_dc_get_visible(dc) != RT_TRUE) return;

	bmp = (struct rtgui_image_bmp*) image->data;
	RT_ASSERT(bmp != RT_NULL);

	/* the minimum rect */
	if (image->w < rtgui_rect_width(*dst_rect)) w = image->w;
	else w = rtgui_rect_width(*dst_rect);
	if (image->h < rtgui_rect_height(*dst_rect)) h = image->h;
	else h = rtgui_rect_height(*dst_rect);

	if (bmp->pixels != RT_NULL)
	{
		rt_uint8_t* ptr;

		/* get pixel pointer */
		ptr = bmp->pixels;
		if (bmp->byte_per_pixel == hw_driver->bits_per_pixel/8)
		{
			for (y = 0; y < h; y ++)
			{
				dc->engine->blit_line(dc, 
					dst_rect->x1, dst_rect->x1 + w,
					dst_rect->y1 + y, 
					ptr);
				ptr += bmp->pitch;
			}
		}
		else
		{
			rt_size_t pitch;
			rt_uint8_t *line_ptr;

			if (image->palette == RT_NULL)
			{
				rtgui_blit_line_func blit_line;
				line_ptr = (rt_uint8_t*) rtgui_malloc((hw_driver->bits_per_pixel/8) * w);
				blit_line = rtgui_blit_line_get(hw_driver->bits_per_pixel/8 , bmp->byte_per_pixel);
				pitch = w * bmp->byte_per_pixel;
				if (line_ptr != RT_NULL)
				{
					for (y = 0; y < h; y ++)
					{
						blit_line(line_ptr, ptr, pitch);
						dc->engine->blit_line(dc,
							dst_rect->x1, dst_rect->x1 + w,
							dst_rect->y1 + y, 
							line_ptr);
						ptr += bmp->pitch;
					}
				}
				rtgui_free(line_ptr);
			}
			else
			{
				int x;
				rtgui_color_t color;

				/* use palette */
				for (y = 0; y < h; y ++)
				{
					ptr = bmp->pixels + (y * bmp->pitch);
					for (x = 0; x < w; x ++)
					{
						color = image->palette->colors[*ptr]; ptr ++;
						rtgui_dc_draw_color_point(dc, dst_rect->x1 + x, dst_rect->y1 + y, color);
					}
				}
			}
		}
	}
	else
	{
		int offset;
		rt_uint8_t *bits;

		/* calculate offset */
		switch (bmp->ExpandBMP)
		{
		case 1:
			offset = (image->h - h) * (image->w/8) * bmp->byte_per_pixel;
			break;

		case 4:
			offset = (image->h - h) * (image->w/2) * bmp->byte_per_pixel;
			break;

		default:
			offset = (image->h - h) * image->w * bmp->byte_per_pixel;
			break;
		}
		/* seek to the begin of pixel data */
		rtgui_filerw_seek(bmp->filerw, bmp->pixel_offset + offset, RTGUI_FILE_SEEK_SET);

		if (bmp->ExpandBMP == 1 || bmp->ExpandBMP == 4)
		{
			int x;
			rtgui_color_t color;
			/* 1, 4 bit per pixels */

			/* draw each line */
			for (y = h - 1; y >= 0; y --)
			{
				/* read pixel data */
				rt_uint8_t pixel = 0;
				int   shift = (8 - bmp->ExpandBMP);
				int i;

				bits = bmp->line_pixels;
				for ( i=0; i < image->w; ++i )
				{
					if ( i % (8/bmp->ExpandBMP) == 0 )
					{
						if ( !rtgui_filerw_read(bmp->filerw, &pixel, 1, 1) )
							return;
					}
					*(bits+i) = (pixel>>shift);
					pixel <<= bmp->ExpandBMP;
				}

				/* Skip padding bytes  */
				if (bmp->pad)
				{
					int i;
					rt_uint8_t padbyte;
					for ( i=0; i < bmp->pad; ++i )
						rtgui_filerw_read(bmp->filerw, &padbyte, 1, 1);
				}

				/* use palette */
				bits = bmp->line_pixels;
				for (x = 0; x < w; x ++)
				{
					color = image->palette->colors[*bits]; bits ++;
					rtgui_dc_draw_color_point(dc, dst_rect->x1 + x, dst_rect->y1 + y, color);
				}
			}
		}
		else
		{
			rt_uint8_t *line_ptr = (rt_uint8_t*) rtgui_malloc((hw_driver->bits_per_pixel/8) * w);
			if (line_ptr == RT_NULL) return;

			/* draw each line */
			for (y = h - 1; y >= 0; y --)
			{
				/* read line pixels */
				rtgui_filerw_read(bmp->filerw, bmp->line_pixels, 1, bmp->pitch);

				/* Skip padding bytes  */
				if (bmp->pad)
				{
					int i;
					rt_uint8_t padbyte;
					for ( i=0; i < bmp->pad; ++i )
						rtgui_filerw_read(bmp->filerw, &padbyte, 1, 1);
				}

				if (image->palette == RT_NULL)
				{
					int pitch;
					rtgui_blit_line_func blit_line;
					blit_line = rtgui_blit_line_get(hw_driver->bits_per_pixel/8 , bmp->byte_per_pixel);
					pitch = w * bmp->byte_per_pixel;
					if (line_ptr != RT_NULL)
					{
						blit_line(line_ptr, bmp->line_pixels, pitch);
						dc->engine->blit_line(dc,
							dst_rect->x1, dst_rect->x1 + w,
							dst_rect->y1 + y, 
							line_ptr);
					}
				}
				else 
				{
					int x;
					rtgui_color_t color;

					/* use palette */
					bits = bmp->line_pixels;
					for (x = 0; x < w; x ++)
					{
						color = image->palette->colors[*bits]; bits ++;
						rtgui_dc_draw_color_point(dc, dst_rect->x1 + x, dst_rect->y1 + y, color);
					}
				}
			}
			if (line_ptr != RT_NULL) rtgui_free(line_ptr);
		}
	}
}

void rtgui_image_bmp_header_cfg(struct rtgui_image_bmp_header *bhr, rt_int32_t w, rt_int32_t h, rt_uint16_t bits_per_pixel)
{
	int image_size = w * h * bits_per_pixel/8;
	int header_size = sizeof(struct rtgui_image_bmp_header);

	bhr->magic[0] = 'B';
	bhr->magic[1] = 'M';
	bhr->bfSize = header_size + image_size; /* data size */
	bhr->bfReserved1 = 0;
	bhr->bfReserved2 = 0;
	bhr->bfOffBits = header_size;

	bhr->biSize = 40; /* sizeof BITMAPINFOHEADER */
	bhr->biWidth = w;
	bhr->biHeight = h;
	bhr->biPlanes = 1;
	bhr->biBitCount = bits_per_pixel;
	bhr->biCompression = BI_BITFIELDS;
	bhr->biSizeImage = image_size;
	bhr->biXPelsPerMeter = 0;
	bhr->biYPelsPerMeter = 0;
	bhr->biClrUsed = 0;
	bhr->biClrImportant = 0;
	if (bhr->biBitCount == 16 && bhr->biCompression == BI_BITFIELDS)
	{
		bhr->bfSize += 12; /* add color mask length. */
		bhr->bfOffBits += 12; /* add color mask length. */
	}
#if (0) /* track log */
	rt_kprintf("bfSize=%d\n",bhr->bfSize);
	rt_kprintf("bfOffBits=%d\n",bhr->bfOffBits);
	rt_kprintf("biWidth=%d\n",bhr->biWidth);
	rt_kprintf("biHeight=%d\n",bhr->biHeight);
	rt_kprintf("biBitCount=%d\n",bhr->biBitCount);
	rt_kprintf("biSizeImage=%d\n",bhr->biSizeImage);
#endif
}

#ifdef _WIN32
#define open	_open
#define close	_close
#define read	_read
#define write	_write
#define unlink	_unlink
#endif

/* #define BGR_CONVERT_TO_RGB */
void bmp_create(const char *filename)
{
	int fd, w, h, i, pitch;
	rt_uint16_t *src;
	rt_uint32_t mask;
	struct rtgui_image_bmp_header bhr;
	struct rtgui_graphic_driver* grp = hw_driver;
#ifdef BGR_CONVERT_TO_RGB
	int j;
	rt_uint16_t *line_buf;
	rt_uint16_t color,tmp;
#endif
	
	fd = open(filename, O_RDWR, 0);
	if(fd >= 0)
	{
		close(fd);
		if (unlink(filename) < 0)
			return;
	}
	fd = open(filename, O_RDWR | O_CREAT, 0);
	if (fd == -1) 
	{
		rt_kprintf("create file failed\n");
		return;
	}

	w = grp->width;
	h = grp->height;

	pitch = w * sizeof(rt_uint16_t);
#ifdef BGR_CONVERT_TO_RGB
	line_buf = rt_malloc(pitch);
	if (line_buf == RT_NULL)
	{
		rt_kprintf("no memory!\n");
		return;
	}
#endif
	rtgui_image_bmp_header_cfg(&bhr, w, h, grp->bits_per_pixel);
	write(fd, &bhr, sizeof(struct rtgui_image_bmp_header));

	if(bhr.biCompression == BI_BITFIELDS)
	{
		mask = 0xF800;  /* Red Mask */
		write(fd, &mask, 4);
		mask = 0x07E0;  /* Green Mask */
		write(fd, &mask, 4);
		mask = 0x001F;  /* Blue Mask */
		write(fd, &mask, 4);
	}

	src = (rt_uint16_t*)grp->framebuffer;
	src += w*h;
	for(i=0; i<h; i++)
	{
		src -= w;
#ifdef BGR_CONVERT_TO_RGB
		for(j=0; j<w; j++)
		{
			tmp = *(src+j);
			//BGR convert to RGB
			color =  (tmp&0x001F)<<11;
			color += (tmp&0x07E0);
			color += (tmp&0xF800)>>11;

			*(line_buf+j) = color;
		}

		write(fd, line_buf, pitch);
#else
		write(fd, src, pitch);
#endif
	}
#ifdef BGR_CONVERT_TO_RGB
	rt_free(line_buf);
#endif
	close(fd);
}

void rtgui_image_bmp_init(void)
{
	/* register bmp on image system */
	rtgui_image_register_engine(&rtgui_image_bmp_engine);
}
#endif
