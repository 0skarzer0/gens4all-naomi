#if defined(DEBUG_VDP) && defined(DEBUG_EXTRA_VDP)

void print_spr_vis(void);
void print_sprite_struct(void);
void print_pattern_data(int data);
void print_pattern_info(int data);
void print_yoffset(int data);
void print_xoffset(unsigned int data);
void print_current_line(void);
void print_render_line_start(void);
void print_debug_putline_sprite(int dest);
void print_special_debug_line_spr(int dest,int spr_idx);
void print_putpixel_sprite(int pattern_data, int first_pat);
void print_first_pat(int first_pat);
void print_pixel_value(int pixel_value, int palnum);
void print_cell_info(int cell_info);
void print_debug_scroll_a(unsigned int dest);
void print_tmp_value_win(void);
void print_offsets(int x, int y);
void print_tag(int x);
void print_putline(unsigned dest,unsigned palette_number);
void print_pixel8(unsigned addr,unsigned char pix);
void print_byte(unsigned char value);
void print_word(unsigned short value);
void print_dword(unsigned value);
#else
#define print_spr_vis()
#define print_sprite_struct()
#define print_pattern_data(d)
#define print_pattern_info(d)
#define print_yoffset(d)
#define print_xoffset(d)
#define print_current_line()
#define print_render_line_start()
#define print_debug_putline_sprite(d)
#define print_special_debug_line_spr(d,s)
#define print_putpixel_sprite(p,f)
#define print_first_pat(f)
#define print_pixel_value(p,m)
#define print_cell_info(c)
#define print_debug_scroll_a(d)
#define print_tmp_value_win()
#define print_offsets(x, y)
#define print_tag(x)
#define print_putline(d, p)
#define print_pixel8(a, p)
#define print_byte(v)
#define print_word(v)
#define print_dword(v)
#endif

#ifdef DEBUG_VDP
void print_mdscreen(void);
#else
#define print_mdscreen()
#endif
