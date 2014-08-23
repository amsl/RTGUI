/*
 * File      : grid.c
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */
#include <string.h>
#include <rtgui/widgets/grid.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/scrollbar.h>

static rt_bool_t rtgui_grid_sbar_handle(pvoid wdt, rtgui_event_t* event);

static void _rtgui_grid_constructor(rtgui_grid_t *grid)
{
	/* set default widget rect and set event handler */
	rtgui_widget_set_event_handler(grid,rtgui_grid_event_handler);
	rtgui_widget_set_onunfocus(grid, rtgui_grid_unfocus);
	RTGUI_WIDGET_FLAG(grid) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	rtgui_widget_set_border_style(grid, RTGUI_BORDER_DOWN);
	grid->first_row = 0;
	grid->frist_col = 0;
	grid->now_row = 0;
	grid->old_row = 0;
	grid->now_col = 0;
	grid->old_col = 0;
	grid->rows = 0;
	grid->cols = 0;
	grid->item_height  = RTGUI_SEL_H;
	grid->row_per_page = 0;
	grid->col_per_page = 0;
	grid->grid_head = 0;	/* 表格头 */
	grid->grid_line = 1;	/* 表格线 */
	grid->grid_color = Gray;
	grid->widget_link = RT_NULL;

	RTGUI_WIDGET_BC(grid) = theme.blankspace;
	RTGUI_WIDGET_TEXTALIGN(grid) = RTGUI_ALIGN_LEFT|RTGUI_ALIGN_CENTER_VERTICAL;

	grid->colw = RT_NULL;
	grid->items = RT_NULL;
	grid->vbar = RT_NULL;
	grid->on_item = RT_NULL;
}
static void _rtgui_grid_destructor(rtgui_grid_t *grid)
{
	if(grid->colw != RT_NULL)
	{
		rt_free(grid->colw);
		grid->colw = RT_NULL;
	}
	if(grid->items != RT_NULL)
	{
		rt_free(grid->items);
		grid->items = RT_NULL;
	}
}

DEFINE_CLASS_TYPE(grid, "grid",
	RTGUI_CONTAINER_TYPE,
	_rtgui_grid_constructor,
	_rtgui_grid_destructor,
	sizeof(struct rtgui_grid));

rtgui_grid_t* rtgui_grid_create(pvoid parent, int left,int top,int w,int h,int rows,int cols)
{
	rtgui_container_t *container;
	rtgui_grid_t* grid = RT_NULL;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	grid = rtgui_widget_create(RTGUI_GRID_TYPE);
	if(grid != RT_NULL)
	{
		rtgui_rect_t rect;
		int i;

		rtgui_widget_get_rect(container,&rect);
		rtgui_widget_rect_to_device(container, &rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(grid,&rect);
		rtgui_container_add_child(container, grid);

		grid->row_per_page = h / (grid->item_height+2);
		grid->col_per_page = 3;
		grid->rows = rows;
		grid->cols = cols;

		grid->items = rt_malloc(rows*cols*sizeof(rtgui_grid_item_t));
		if(grid->items  != RT_NULL)
			memset(grid->items, 0, rows*cols*sizeof(rtgui_grid_item_t));
		grid->colw = rt_malloc(cols*sizeof(rt_uint16_t));
		for(i=0; i<grid->cols; i++)
		{	/* 默认列宽等分 */
			*(grid->colw+i) = (w-RTGUI_WIDGET_BORDER_SIZE(grid)*2)/grid->cols;
		}

		if(grid->vbar == RT_NULL)
		{
			/* create scrollbar */
			rt_uint32_t sl,st,sw=RTGUI_DEFAULT_SB_WIDTH,slen;
			sl = RC_W(rect)-RTGUI_WIDGET_BORDER_SIZE(grid)-sw;
			st = RTGUI_WIDGET_BORDER_SIZE(grid);
			slen = RC_H(rect)-RTGUI_WIDGET_BORDER_SIZE(grid)*2;

			grid->vbar = rtgui_scrollbar_create(grid,sl,st,sw,slen,RTGUI_VERTICAL);

			if(grid->vbar != RT_NULL)
			{
				grid->vbar->widget_link = (pvoid)grid;
				grid->vbar->on_scroll = rtgui_grid_sbar_handle;

				if(grid->rows > grid->row_per_page)
				{
					RTGUI_WIDGET_SHOW(grid->vbar);
					rtgui_scrollbar_set_line_step(grid->vbar, 1);
					rtgui_scrollbar_set_page_step(grid->vbar, grid->row_per_page);
					rtgui_scrollbar_set_range(grid->vbar, grid->rows);
				}
				else
				{
					RTGUI_WIDGET_HIDE(grid->vbar);
				}
			}
		}
	}

	return grid;
}

void rtgui_grid_destroy(rtgui_grid_t* grid)
{
	/* destroy grid */
	rtgui_widget_destroy(grid);
}

/* draw grid all item */
void rtgui_grid_ondraw(rtgui_grid_t* grid)
{
	rtgui_rect_t rect, item_rect;
	rt_uint16_t first, col,row;
	int left;
	const rtgui_grid_item_t* item;
	rtgui_dc_t* dc;

	RT_ASSERT(grid != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(grid);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(grid, &rect);

	/* draw grid border */
	rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(grid));
	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(grid));
	RTGUI_DC_BC(dc) = theme.blankspace;
	rtgui_dc_fill_rect(dc, &rect);
	rtgui_rect_inflate(&rect,RTGUI_WIDGET_BORDER_SIZE(grid));

	if(grid->items==RT_NULL)return;/* not exist items. */

	if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid->vbar))
	{
		rect.x2 -= RC_W(grid->vbar->parent.extent);
	}

	/* get grid base rect */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.y2 = item_rect.y1 + (grid->item_height+2);
	left = item_rect.x1;

	/* get first row */
	first = grid->first_row;

	for(row = 0; row < grid->row_per_page; row++)
	{
		item_rect.x1 = left;
		if(first + row >= grid->rows) break;

		for(col = 0; col < grid->cols; col ++)
		{
			item = grid->items+((first+row)*grid->cols+col);
			item_rect.x2 = item_rect.x1 + *(grid->colw+col);

			if(first+row==grid->now_row && col==grid->now_col)
			{
				//draw current grid
				if(RTGUI_WIDGET_IS_FOCUSED(grid))
				{
					RTGUI_DC_BC(dc) = DarkBlue;
					RTGUI_DC_FC(dc) = theme.blankspace;
					if(row != 0) item_rect.y1 += 1;
					if(col != 0) item_rect.x1 += 1;
					rtgui_dc_fill_rect(dc, &item_rect);
					if(row != 0) item_rect.y1 -= 1;
					if(col != 0) item_rect.x1 -= 1;
				}
				else
				{
					RTGUI_DC_BC(dc) = Gray;
					RTGUI_DC_FC(dc) = theme.foreground;
					if(row != 0) item_rect.y1 += 1;
					if(col != 0) item_rect.x1 += 1;
					rtgui_dc_fill_rect(dc, &item_rect);
					if(row != 0) item_rect.y1 -= 1;
					if(col != 0) item_rect.x1 -= 1;
				}
			}
			if(grid->grid_line)
			{
				RTGUI_DC_FC(dc) = grid->grid_color;
				rtgui_dc_draw_line(dc,item_rect.x2,item_rect.y1,item_rect.x2,item_rect.y2);
				rtgui_dc_draw_line(dc,item_rect.x1,item_rect.y2,item_rect.x2+1,item_rect.y2);
			}

			item_rect.x1 += RTGUI_MARGIN;
			/* draw text */
			if(first+row==grid->now_row && col==grid->now_col && RTGUI_WIDGET_IS_FOCUSED(grid))
			{
				RTGUI_DC_FC(dc) = theme.blankspace;
				if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
			}
			else
			{
				RTGUI_DC_FC(dc) = theme.foreground;
				if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
			}

			item_rect.x1 = item_rect.x2;
		}
		/* move to next item position */
		item_rect.y1 += (grid->item_height + 2);
		item_rect.y2 += (grid->item_height + 2);
	}

	if(grid->vbar)
	{
		if(!RTGUI_WIDGET_IS_HIDE(grid->vbar))
			rtgui_scrollbar_ondraw(grid->vbar);
	}

	rtgui_dc_end_drawing(dc);
}

/* update grid new/old focus item */
void rtgui_grid_update(rtgui_grid_t* grid)
{
	int c;
	const rtgui_grid_item_t* item;
	rtgui_rect_t rect, item_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(grid != RT_NULL);

	if(RTGUI_WIDGET_IS_HIDE(grid))return;
	if(grid->items==RT_NULL)return;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(grid);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(grid, &rect);

	if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid->vbar))
	{
		rect.x2 -= RC_W(grid->vbar->parent.extent);
	}

	if(((grid->old_row >= grid->first_row) && /* int front some page */
	        (grid->old_row < grid->first_row+grid->row_per_page) && /* int later some page */
	        (grid->old_row != grid->now_row)) || /* change location */
	        (grid->old_col != grid->now_col))
	{
		/* these condition dispell blinked when drawed */
		item_rect = rect;
		/* get old item's rect */
		item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(grid);
		for(c=0; c<grid->old_col; c++)
			item_rect.x1 += *(grid->colw+c);
		item_rect.x2 = item_rect.x1 + *(grid->colw+grid->old_col);
		item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(grid);
		item_rect.y1 += ((grid->old_row-grid->first_row) % grid->row_per_page) * (grid->item_height+2);
		item_rect.y2 = item_rect.y1 + (grid->item_height+2);

		/* draw old item */
		RTGUI_DC_BC(dc) = theme.blankspace;
		RTGUI_DC_FC(dc) = theme.foreground;
		if(grid->old_row != 0) item_rect.y1 += 1;
		if(grid->old_col != 0) item_rect.x1 += 1;
		rtgui_dc_fill_rect(dc,&item_rect);
		if(grid->old_row != 0) item_rect.y1 -= 1;
		if(grid->old_col != 0) item_rect.x1 -= 1;

		item_rect.x1 += RTGUI_MARGIN;
		item = grid->items + (grid->old_row*grid->cols+grid->old_col);
		if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
		if(grid->grid_line)
		{
			RTGUI_DC_FC(dc) = grid->grid_color;
			rtgui_dc_draw_line(dc, item_rect.x2, item_rect.y1,item_rect.x2,item_rect.y2);
			rtgui_dc_draw_line(dc, item_rect.x1, item_rect.y2,item_rect.x2+1,item_rect.y2);
		}
	}

	/* draw now item */
	item_rect = rect;
	/* get now item's rect */
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(grid);
	for(c=0; c<grid->now_col; c++)
		item_rect.x1 += *(grid->colw+c);
	item_rect.x2 = item_rect.x1 + *(grid->colw+grid->now_col);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.y1 += ((grid->now_row-grid->first_row) % grid->row_per_page) * (grid->item_height+2);
	item_rect.y2 = item_rect.y1 + (2 + grid->item_height);

	/* draw current item */
	if(RTGUI_WIDGET_IS_FOCUSED(grid))
	{
		RTGUI_DC_BC(dc) = DarkBlue;
		RTGUI_DC_FC(dc) = theme.blankspace;
		if(grid->now_row != 0) item_rect.y1 += 1;
		if(grid->now_col != 0) item_rect.x1 += 1;
		rtgui_dc_fill_rect(dc, &item_rect);
		if(grid->now_row != 0) item_rect.y1 -= 1;
		if(grid->now_col != 0) item_rect.x1 -= 1;
	}
	else
	{
		RTGUI_DC_BC(dc) = Gray;
		RTGUI_DC_FC(dc) = theme.foreground;
		if(grid->now_row != 0 && grid->now_col != 0)
		{
			item_rect.x1 += 1;
			item_rect.y1 += 1;
		}
		rtgui_dc_fill_rect(dc, &item_rect);
		if(grid->now_row != 0 && grid->now_col != 0)
		{
			item_rect.x1 -= 1;
			item_rect.y1 -= 1;
		}
	}

	item_rect.x1 += RTGUI_MARGIN;
	item = grid->items + (grid->now_row*grid->cols+grid->now_col);

	if(RTGUI_WIDGET_IS_FOCUSED(grid))
	{
		RTGUI_DC_FC(dc) = theme.blankspace;
		if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
	}
	else
	{
		RTGUI_DC_FC(dc) = theme.foreground;
		if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
	}

	rtgui_dc_end_drawing(dc);
}

static void rtgui_grid_onmouse(rtgui_grid_t* grid, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect;

	/* get physical extent information */
	rtgui_widget_get_rect(grid, &rect);
	if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid->vbar))
		rect.x2 -= RC_W(grid->vbar->parent.extent);

	if((rtgui_region_contains_point(&RTGUI_WIDGET_CLIP(grid), emouse->x, emouse->y,&rect) == RT_EOK)
	        && (grid->rows > 0) && (grid->cols > 0))
	{
		rt_int16_t c,row=0,col=0;
		rtgui_rect_t t_rect=rect;

		/* set focus */
		rtgui_widget_focus(grid);

		/* get row */
		row = (emouse->y - rect.y1) / (grid->item_height+2);
		/* negative invalid */
		if(row < 0) return;
		/* outside range invalid */
		if((row+grid->first_row) >= grid->rows) return;

		/* get col */
		for(c=0; c<grid->cols; c++)
		{
			t_rect.x2 = t_rect.x1 + *(grid->colw+c);
			if(emouse->x>=t_rect.x1 && emouse->x<=t_rect.x2)
			{
				col = c;
				break;
			}
			t_rect.x1 = t_rect.x2;
		}
		//在无效区
		if(c >= grid->cols) return;

		if((row < grid->rows) && (row < grid->row_per_page) && (col < grid->cols))
		{
			if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
			{
				grid->old_row = grid->now_row;
				grid->now_row = grid->first_row + row;
				grid->old_col = grid->now_col;
				grid->now_col = grid->frist_col + col;

				if(grid->on_item != RT_NULL)
				{
					grid->on_item(grid, (rtgui_event_t*)emouse);
				}

				/* down event */
				rtgui_grid_update(grid);
			}
			else if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
			{
				rtgui_grid_update(grid);
			}
		}
	}
}

rt_bool_t rtgui_grid_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_grid_t* grid = RTGUI_GRID(wdt);

	RT_ASSERT(grid != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw)
			widget->on_draw(widget, event);
		else
			rtgui_grid_ondraw(grid);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
		{
			rtgui_grid_onmouse(grid, (struct rtgui_event_mouse*)event);
		}
		rtgui_container_dispatch_mouse_event(grid, (struct rtgui_event_mouse*)event);
		return RT_TRUE;

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd*)event;
		if((RTGUI_KBD_IS_DOWN(ekbd)) && (grid->rows > 0))
		{
			switch(ekbd->key)
			{
			case RTGUIK_LEFT:
				if(grid->now_col > 0)
				{
					grid->old_row = grid->now_row;
					grid->old_col = grid->now_col;
					grid->now_col --;
					if(grid->now_col < grid->frist_col)
					{
						if(grid->frist_col)grid->frist_col--;
						rtgui_grid_ondraw(grid);
					}
					else
					{
						rtgui_grid_update(grid);
					}
				}
				break;

			case RTGUIK_RIGHT:
				if(grid->now_col < grid->cols -1)
				{
					grid->old_row = grid->now_row;
					grid->old_col = grid->now_col;
					grid->now_col ++;
					if(grid->now_col >= grid->frist_col+grid->col_per_page)
					{
						grid->frist_col++;
						rtgui_grid_ondraw(grid);
					}
					else
					{
						rtgui_grid_update(grid);
					}

					/*if(grid->hbar && !RTGUI_WIDGET_IS_HIDE(grid))
					{
						rtgui_scrollbar_set_value(grid->hbar,grid->frist_col);
					}*/
				}
				break;

			case RTGUIK_UP:
				if(grid->now_row > 0)
				{
					grid->old_row = grid->now_row;
					grid->old_col = grid->now_col;
					grid->now_row --;
					if(grid->now_row < grid->first_row)
					{
						if(grid->first_row)grid->first_row--;
						rtgui_grid_ondraw(grid);
					}
					else
					{
						rtgui_grid_update(grid);
					}

					if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid))
					{
						rtgui_scrollbar_set_value(grid->vbar,grid->first_row);
					}
					if(grid->on_item != RT_NULL)
					{
						grid->on_item(grid, event);
					}
				}
				break;

			case RTGUIK_DOWN:
				if(grid->now_row < grid->rows - 1)
				{
					grid->old_row = grid->now_row;
					grid->old_col = grid->now_col;
					grid->now_row ++;
					if(grid->now_row >= grid->first_row+grid->row_per_page)
					{
						grid->first_row++;
						rtgui_grid_ondraw(grid);
					}
					else
					{
						rtgui_grid_update(grid);
					}
					if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid))
					{
						rtgui_scrollbar_set_value(grid->vbar,grid->first_row);
					}
					if(grid->on_item != RT_NULL)
					{
						grid->on_item(grid, event);
					}
				}
				break;

			case RTGUIK_RETURN:
				if(grid->on_item != RT_NULL)
				{
					grid->on_item(grid, event);
				}
				break;
			case RTGUIK_BACKSPACE:
				break;

			default:
				break;
			}
		}
		return RT_TRUE;
	}
	default:
		return rtgui_container_event_handler(grid, event);
	}
}

void rtgui_grid_set_onitem(rtgui_grid_t* grid, rtgui_event_handler_ptr func)
{
	RT_ASSERT(grid != RT_NULL);

	grid->on_item = func;
}

/* adjust&update scrollbar widget value */
void rtgui_grid_adjust_vbar(rtgui_grid_t* grid)
{
	if(grid->vbar != RT_NULL)
	{
		rtgui_panel_t *panel = rtgui_panel_get();
		if(grid->rows > grid->row_per_page)
		{
			RTGUI_WIDGET_SHOW(grid->vbar);
			rtgui_widget_update_clip(grid);
			rtgui_scrollbar_set_line_step(grid->vbar, 1);
			rtgui_scrollbar_set_page_step(grid->vbar, grid->row_per_page);
			rtgui_scrollbar_set_range(grid->vbar, grid->rows);
			rtgui_scrollbar_ondraw(grid->vbar);
		}
		else
		{
			if(!RTGUI_WIDGET_IS_HIDE(grid->vbar))
			{
				rtgui_scrollbar_hide(grid->vbar);
				rtgui_widget_update_clip(grid);
			}
		}
	}
}

void rtgui_grid_set_grid(rtgui_grid_t *grid, int row, int col, char *text, int update)
{
	rtgui_grid_item_t *item;

	RT_ASSERT(grid != RT_NULL);

	if(grid->rows==0 || grid->cols==0) return;
	if(row>=grid->rows) return;
	if(col>=grid->cols) return;

	item = grid->items + (row*grid->cols+col);
	item->name = rt_strdup(text);

	if(update) rtgui_grid_update_grid(grid, row, col);
}

void rtgui_grid_set_colw(rtgui_grid_t* grid, int col, int width)
{
	RT_ASSERT(grid != RT_NULL);

	if(col>=grid->cols) return;

	*(grid->colw+col) = width;
}

int rtgui_grid_get_rows(rtgui_grid_t* grid)
{
	RT_ASSERT(grid != RT_NULL);
	return grid->rows;
}

int rtgui_grid_get_cols(rtgui_grid_t* grid)
{
	RT_ASSERT(grid != RT_NULL);
	return grid->cols;
}

/* append at the end of list */
void rtgui_grid_append_row(rtgui_grid_t* grid, int update)
{
	rtgui_grid_item_t *item,*items=RT_NULL;
	int col;

	if(grid->rows >= 65536) return;

	grid->rows += 1;
	if(grid->items)
		items = rt_realloc(grid->items, grid->rows*grid->cols*sizeof(rtgui_grid_item_t));
	else
		items = rt_malloc(grid->rows*grid->cols*sizeof(rtgui_grid_item_t));
	if(items != RT_NULL) grid->items = items;

	for(col=0; col<grid->cols; col++)
	{
		/* initialize new grid value */
		item = grid->items + (grid->rows-1)*grid->cols+col;
		item->name = RT_NULL;
		if(update) rtgui_grid_update_grid(grid, grid->rows-1, col);
	}

	rtgui_grid_adjust_vbar(grid);
}

/* delete current row, by now_row */
void rtgui_grid_delete_row(rtgui_grid_t* grid, int update)
{
	rtgui_grid_item_t *item,*item2,*items=RT_NULL;
	int row,col;

	if(grid->rows <= 0)return;

	/* 当前行是显示的第一行,也是表格的最后一行,并且前面还有数据 */
	if(grid->now_row == grid->first_row && grid->now_row == (grid->rows-1) &&
	        grid->rows>1)
	{
		/* 将显示区域前移 */
		if(grid->first_row>grid->row_per_page)
			grid->first_row -= grid->row_per_page;
		else
			grid->first_row = 0;
		rtgui_grid_ondraw(grid);
	}

	/* 清空当前行的内容 */
	for(col=0; col<grid->cols; col++)
	{
		item = grid->items + (grid->now_row)*grid->cols+col;
		if(item->name != RT_NULL)
		{
			rt_free(item->name);
			item->name = RT_NULL;
		}
	}
	/* 将后面的数据向前移动 */
	if(grid->now_row < (grid->rows-1))
	{
		for(row=grid->now_row; row<grid->rows-1; row++)
		{
			for(col=0; col<grid->cols; col++)
			{
				item  = grid->items + row*grid->cols+col;
				item2 = grid->items + (row+1)*grid->cols+col;
				item->name = item2->name;
				rtgui_grid_update_grid(grid,row-grid->first_row,col);
			}
		}
	}
	/* 清空最后一行的内容 */
	for(col=0; col<grid->cols; col++)
	{
		item = grid->items + (grid->rows-1)*grid->cols+col;
		item->name = RT_NULL;
		rtgui_grid_clear_grid(grid,(grid->rows-1)-grid->first_row,col);
	}

	if(grid->now_row >= (grid->rows-1))
	{
		/* change focus grid */
		if(grid->now_row>0)grid->now_row--;
		grid->old_row = grid->now_row;
		rtgui_grid_update_grid(grid,grid->now_row-grid->first_row,col);
	}

	grid->rows -= 1;
	items = rt_realloc(grid->items, grid->rows*grid->cols*sizeof(rtgui_grid_item_t));
	grid->items = items;

	/* adjust scrollbar value */
	if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid->vbar))
	{
		rtgui_scrollbar_set_value(grid->vbar,grid->rows-grid->row_per_page);
	}
	rtgui_grid_adjust_vbar(grid);
}

void rtgui_grid_insert_row(rtgui_grid_t* grid, int update)
{
	rtgui_grid_item_t *item,*item2;
	int row,col;

	rtgui_grid_append_row(grid, update); /* add new row at the end of list */

	for(row=grid->rows-1; row>grid->now_row; row--)
	{
		for(col=0; col<grid->cols; col++)
		{
			item  = grid->items + row*grid->cols+col;
			item2 = grid->items + (row-1)*grid->cols+col;
			item->name = item2->name;
			rtgui_grid_update_grid(grid,row-grid->first_row,col);
		}
	}

	for(col=0; col<grid->cols; col++)
	{
		item = grid->items + grid->now_row*grid->cols+col;
		item->name = RT_NULL;
		rtgui_grid_update_grid(grid,grid->now_row-grid->first_row,col);
	}

	rtgui_grid_adjust_vbar(grid);

	if(update) rtgui_grid_ondraw(grid);
}

void rtgui_grid_update_grid(rtgui_grid_t* grid, int row, int col)
{
	int i,first;
	rtgui_grid_item_t *item;
	rtgui_rect_t rect,item_rect;
	rtgui_dc_t *dc;

	if(grid->items==RT_NULL)return;/* not exist items. */
	if(row >= grid->rows) return;
	if(col >= grid->cols) return;

	/* get first row */
	first = grid->first_row;
	if(first + row >= grid->rows) return;
	if(row >= grid->row_per_page) return;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(grid);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(grid, &rect);
	if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid->vbar))
	{
		rect.x2 -= RC_W(grid->vbar->parent.extent);
	}

	/* get grid base rect */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(grid) + row*(grid->item_height+2);
	item_rect.y2 = item_rect.y1 + (grid->item_height+2);

	for(i = 0; i < grid->cols; i ++)
	{
		item_rect.x2 = item_rect.x1 + *(grid->colw+i);
		if(i==col)break;
		item_rect.x1 = item_rect.x2;
	}

	item = grid->items + (first+row)*grid->cols + col;

	if(first+row==grid->now_row && col==grid->now_col)
	{
		//draw current grid
		if(RTGUI_WIDGET_IS_FOCUSED(grid))
		{
			RTGUI_DC_BC(dc) = DarkBlue;
			RTGUI_DC_FC(dc) = theme.blankspace;
			if(row != 0) item_rect.y1 += 1;
			if(col != 0) item_rect.x1 += 1;
			rtgui_dc_fill_rect(dc, &item_rect);
			if(row != 0) item_rect.y1 -= 1;
			if(col != 0) item_rect.x1 -= 1;
		}
		else
		{
			RTGUI_DC_BC(dc) = Gray;
			RTGUI_DC_FC(dc) = theme.foreground;
			if(row != 0) item_rect.y1 += 1;
			if(col != 0) item_rect.x1 += 1;
			rtgui_dc_fill_rect(dc, &item_rect);
			if(row != 0) item_rect.y1 -= 1;
			if(col != 0) item_rect.x1 -= 1;
		}
	}
	else
	{
		RTGUI_DC_BC(dc) = theme.blankspace;
		RTGUI_DC_FC(dc) = theme.foreground;
		if(row != 0) item_rect.y1 += 1;
		if(col != 0) item_rect.x1 += 1;
		rtgui_dc_fill_rect(dc, &item_rect);
		if(row != 0) item_rect.y1 -= 1;
		if(col != 0) item_rect.x1 -= 1;
	}
	if(grid->grid_line)
	{
		RTGUI_DC_FC(dc) = grid->grid_color;
		rtgui_dc_draw_line(dc,item_rect.x2,item_rect.y1,item_rect.x2,item_rect.y2);
		rtgui_dc_draw_line(dc,item_rect.x1,item_rect.y2,item_rect.x2+1,item_rect.y2);
	}

	item_rect.x1 += RTGUI_MARGIN;
	/* draw text */
	if(first+row==grid->now_row && col==grid->now_col && RTGUI_WIDGET_IS_FOCUSED(grid))
	{
		RTGUI_DC_FC(dc) = theme.blankspace;
		if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
	}
	else
	{
		RTGUI_DC_FC(dc) = theme.foreground;
		if(item->name) rtgui_dc_draw_text(dc, item->name, &item_rect);
	}

	rtgui_dc_end_drawing(dc);
}

/* clear grid area,fill background color */
void rtgui_grid_clear_grid(rtgui_grid_t *grid, int row, int col)
{
	int i;
	rtgui_rect_t rect,item_rect;
	rtgui_dc_t *dc;

	if(row >= grid->row_per_page) return;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(grid);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(grid, &rect);
	if(grid->vbar && !RTGUI_WIDGET_IS_HIDE(grid->vbar))
	{
		rect.x2 -= RC_W(grid->vbar->parent.extent);
	}

	/* get grid base rect */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(grid);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(grid) + row*(grid->item_height+2);
	item_rect.y2 = item_rect.y1 + (grid->item_height+2);

	for(i = 0; i < grid->cols; i ++)
	{
		item_rect.x2 = item_rect.x1 + *(grid->colw+i);
		if(i==col)break;
		item_rect.x1 = item_rect.x2;
	}

	RTGUI_DC_BC(dc) = theme.blankspace;
	if(row != 0)item_rect.y1 += 1;
	if(col != 0)item_rect.x1 += 1;
	item_rect.x2 += 1;
	item_rect.y2 += 1;
	rtgui_dc_fill_rect(dc, &item_rect);
	rtgui_dc_end_drawing(dc);
}

void rtgui_grid_clear_items(rtgui_grid_t* grid)
{
	int row,col;
	rtgui_grid_item_t* item;

	if(grid->items != RT_NULL)
	{
		for(row=0; row<grid->rows; row++)
		{
			for(col=0; col<grid->cols; col++)
			{
				item = grid->items + row*grid->cols + col;
				if(item->name != RT_NULL)
				{
					rt_free(item->name);
					item->name = RT_NULL;
				}
			}
		}
		rt_free(grid->items);
		grid->items = RT_NULL;
		grid->rows = 0;
		grid->first_row = 0;
		grid->frist_col = 0;
		grid->now_row = 0;
		grid->old_row = 0;
		grid->now_col = 0;
		grid->old_col = 0;
		rtgui_grid_adjust_vbar(grid);
	}
}

rt_bool_t rtgui_grid_unfocus(pvoid wdt, rtgui_event_t* event)
{
	rtgui_grid_t *grid = RTGUI_GRID(wdt);
	if(grid == RT_NULL)return RT_FALSE;

	if(!RTGUI_WIDGET_IS_FOCUSED(grid))
	{
		/* clear focus rectangle */
		rtgui_grid_update(grid);
	}

	return RT_TRUE;
}

static rt_bool_t rtgui_grid_sbar_handle(pvoid wdt, rtgui_event_t* event)
{
	rtgui_grid_t *grid = RTGUI_GRID(wdt);

	if (grid->first_row == grid->vbar->value) return RT_FALSE;
	
	grid->first_row = grid->vbar->value;
	rtgui_grid_ondraw(grid);

	return RT_TRUE;
}

