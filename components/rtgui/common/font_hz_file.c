/*
 * Cached HZ font engine
 */
#include <rtgui/dc.h>
#ifdef RTGUI_USING_HZ_FILE
#include <dfs_posix.h>

#define HZ_CACHE_MAX    64

static int _font_cache_compare(struct hz_cache* node1, struct hz_cache* node2);

static void rtgui_hz_file_font_load(struct rtgui_font* font);
static void rtgui_hz_file_font_draw_text(struct rtgui_font* font, rtgui_dc_t *dc, const char* text, rt_uint32_t len, rtgui_rect_t* rect);
static void rtgui_hz_file_font_get_metrics(struct rtgui_font* font, const char* text, rtgui_rect_t* rect);
const struct rtgui_font_engine rtgui_hz_file_font_engine =
{
	RT_NULL,
	rtgui_hz_file_font_load,
	rtgui_hz_file_font_draw_text,
	rtgui_hz_file_font_get_metrics
};

SPLAY_PROTOTYPE(cache_tree, hz_cache, hz_node, _font_cache_compare);
SPLAY_GENERATE(cache_tree, hz_cache, hz_node, _font_cache_compare);

static int _font_cache_compare(struct hz_cache* cache_1, struct hz_cache* cache_2)
{
	if(cache_1->hz_id > cache_2->hz_id) return 1;
	if(cache_1->hz_id < cache_2->hz_id) return -1;

	return 0;
}

static rt_uint8_t* _font_cache_get(struct rtgui_hz_file_font* font, rt_uint16_t hz_id)
{
	rt_uint32_t seek;
	struct hz_cache *cache, search;

	search.hz_id = hz_id;
	cache = SPLAY_FIND(cache_tree, &(font->cache_root), &search);
	if(cache != RT_NULL)
	{
		/* find it */
		return (rt_uint8_t*)(cache + 1);
	}

	/* can not find it, load to cache */
	cache = (struct hz_cache*) rt_malloc(sizeof(struct hz_cache) + font->font_data_size);
	if(cache == RT_NULL) return RT_NULL; /* no memory yet */

	cache->hz_id = hz_id;
	seek = 94 * (((hz_id & 0xff) - 0xA0) - 1) + ((hz_id >> 8) - 0xA0) - 1;
	seek *= font->font_data_size;

	/* read hz font data */
	if((lseek(font->fd, seek, SEEK_SET) < 0) ||
	        read(font->fd, (char*)(cache + 1), font->font_data_size) !=
	        font->font_data_size)
	{
		rt_free(cache);
		return RT_NULL;
	}

	/* insert to cache */
	SPLAY_INSERT(cache_tree, &(font->cache_root), cache);
	font->cache_size ++;

	if(font->cache_size > HZ_CACHE_MAX)
	{
		/* remove a cache */
		struct hz_cache* left;
		left = font->cache_root.sph_root;
		while(SPLAY_LEFT(left, hz_node) != RT_NULL) left = SPLAY_LEFT(left, hz_node);

		/* remove the left node */
		SPLAY_REMOVE(cache_tree, &(font->cache_root), left);
		font->cache_size --;
	}

	return (rt_uint8_t*)(cache + 1);
}

static void rtgui_hz_file_font_load(struct rtgui_font* font)
{
	struct rtgui_hz_file_font* hz_file_font = (struct rtgui_hz_file_font*)font->data;
	RT_ASSERT(hz_file_font != RT_NULL);

	hz_file_font->fd = open(hz_file_font->font_fn, O_RDONLY, 0);
}

static void rtgui_hz_file_font_draw_text(struct rtgui_font* font, rtgui_dc_t *dc, const char* text, rt_uint32_t len, rtgui_rect_t* rect)
{
	register rt_int32_t h, word_bytes;
	rt_uint8_t* str;
	struct rtgui_hz_file_font* hz_file_font = (struct rtgui_hz_file_font*)font->data;
	RT_ASSERT(hz_file_font != RT_NULL);

	/* drawing height */
	h = (hz_file_font->font_size + rect->y1 > rect->y2)?
	    rect->y2 - rect->y1 : hz_file_font->font_size;
	word_bytes = (hz_file_font->font_size + 7) / 8;

	str = (rt_uint8_t*)text;

	while(len > 0 && rect->x1 < rect->x2)
	{
		const rt_uint8_t* font_ptr;
		register rt_int32_t i, j, k;

		/* get font pixel data */
		font_ptr = _font_cache_get(hz_file_font, *str | (*(str+1) << 8));

		/* draw word */
		for(i=0; i < h; i ++)
		{
			for(j=0; j < word_bytes; j++)
				for(k=0; k < 8; k++)
				{
					if(((font_ptr[i*2 + j] >> (7-k)) & 0x01) != 0 &&
					        (rect->x1 + 8 * j + k < rect->x2))
					{
						rtgui_dc_draw_color_point(dc, rect->x1 + 8*j + k, rect->y1 + i, color);
					}
				}
		}

		/* move x to next character */
		rect->x1 += hz_file_font->font_size;
		str += 2;
		len -= 2;
	}
}

static void rtgui_hz_file_font_get_metrics(struct rtgui_font* font, const char* text, rtgui_rect_t* rect)
{
	struct rtgui_hz_file_font* hz_file_font = (struct rtgui_hz_file_font*)font->data;
	RT_ASSERT(hz_file_font != RT_NULL);

	/* set metrics rect */
	rect->x1 = rect->y1 = 0;
	rect->x2 = (rt_int16_t)(hz_file_font->font_size/2 * rt_strlen((const char*)text));
	rect->y2 = hz_file_font->font_size;
}
#endif
