%include "nasmhead.inc"

%define HIGH_B   0x80
%define SHAD_B   0x40
%define PRIO_B   0x01
%define SPR_B    0x20

%define HIGH_W   0x8080
%define SHAD_W   0x4040
%define NOSHAD_W 0xBFBF
%define PRIO_W   0x0100
%define SPR_W    0x2000

%define SHAD_D   0x40404040
%define NOSHAD_D 0xBFBFBFBF


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%ifdef DEBUG_VDP
extern print_mdscreen
%endif
%ifdef DEBUG_EXTRA_VDP
extern print_pattern_info
extern print_yoffset
extern print_xoffset
extern print_render_line_start
extern print_pattern_data
extern print_debug_putline_sprite
extern print_special_debug_line_spr
extern print_spr_vis
extern print_sprite_struct
extern print_putpixel_sprite
extern print_first_pat
extern print_pixel_value
extern print_cell_info
extern print_debug_scroll_a
extern print_tmp_value_win
extern print_offsets
extern print_tag
extern print_putline
extern print_pixel8
extern print_byte
extern print_word
extern print_dword
%endif


%macro PRINT_OFFSETS 0
%ifdef DEBUG_EXTRA_VDP
pushad
push edi
push esi
call print_offsets
pop esi
pop edi
popad
%endif
%endmacro

%macro PRINT_PUTLINE 0
%ifdef DEBUG_EXTRA_VDP
pushad
push edx
push ebp
call print_putline
pop ebp
pop edx
popad
%endif
%endmacro

%macro PRINT_TMP_VALUE_WIN 0
%ifdef DEBUG_EXTRA_VDP
pushad
call print_tmp_value_win
popad
%endif
%endmacro

%macro PRINT_DEBUG_SCROLL_A 0
%ifdef DEBUG_EXTRA_VDP
pushad
push ebp
call print_debug_scroll_a
pop ebp
popad
%endif
%endmacro

%macro PRINT_PATTERN_INFO 0
%ifdef DEBUG_EXTRA_VDP
pushad
push eax
call print_pattern_info
pop eax
popad
%endif
%endmacro

%macro PRINT_YOFFSET 0
%ifdef DEBUG_EXTRA_VDP
pushad
push edi
call print_yoffset
pop edi
popad
%endif
%endmacro

%macro PRINT_XOFFSET 0
%ifdef DEBUG_EXTRA_VDP
pushad
push esi
call print_xoffset
pop esi
popad
%endif
%endmacro

%macro PRINT_RENDER_LINE_START 0
%ifdef DEBUG_EXTRA_VDP
pushad
call print_render_line_start
popad
%endif
%endmacro

%macro PRINT_PATTERN_DATA 0
%ifdef DEBUG_EXTRA_VDP
pushad
push ebx
call print_pattern_data
pop ebx
popad
%endif
%endmacro

%macro PRINT_DEBUG_PUTLINE_SPRITE 0
%ifdef DEBUG_EXTRA_VDP
pushad
push ebp
call print_debug_putline_sprite
pop ebp
popad
%endif
%endmacro

%macro PRINT_SPECIAL_DEBUG_LINE_SPR 0
%ifdef DEBUG_EXTRA_VDP
pushad
push edi
push ebp
call print_special_debug_line_spr
pop ebp
pop edi
popad
%endif
%endmacro

%macro PRINT_SPRITE_VISIBLE 0
%ifdef DEBUG_EXTRA_VDP
pushad
call print_spr_vis
popad
%endif
%endmacro

%macro PRINT_MDSCREEN 0
%ifdef DEBUG_VDP
pushad
call print_mdscreen
popad
%endif
%endmacro

%macro PRINT_SPRITE_STRUCT 0
%ifdef DEBUG_EXTRA_VDP
pushad
call print_sprite_struct
popad
%endif
%endmacro

%macro PRINT_PUTPIXEL_SPRITE 0
%ifdef DEBUG_EXTRA_VDP
pushad
push esi
push ebx
call print_putpixel_sprite
pop ebx
pop esi
popad
%endif
%endmacro

%macro PRINT_FIRST_PAT 0
%ifdef DEBUG_EXTRA_VDP
pushad
push esi
call print_first_pat
pop esi
popad
%endif
%endmacro

%macro PRINT_PIXEL_VALUE 0
%ifdef DEBUG_EXTRA_VDP
pushad
push eax
push ecx
call print_pixel_value
pop ecx
pop eax
popad
%endif
%endmacro

%macro PRINT_CELL_INFO 0
%ifdef DEBUG_EXTRA_VDP
pushad
push eax
call print_cell_info
pop eax
popad
%endif
%endmacro

%macro PRINT_TAG 1
%ifdef DEBUG_EXTRA_VDP
pushad
mov eax, %1
push eax
call print_tag
pop eax
popad
%endif
%endmacro

%macro PRINT_PIXEL8 1
%ifdef DEBUG_EXTRA_VDP
pushad
add ebp,%1
shl ebp,1
push eax
push ebp
call print_pixel8
pop ebp
pop eax
popad
%endif
%endmacro

%macro PRINT_BYTE 1
%ifdef DEBUG_EXTRA_VDP
pushad
mov al,%1
push eax
call print_byte
pop eax
popad
%endif
%endmacro

%macro PRINT_WORD 1
%ifdef DEBUG_EXTRA_VDP
pushad
mov ax,%1
push eax
call print_word
pop eax
popad
%endif
%endmacro

%macro PRINT_DWORD 1
%ifdef DEBUG_EXTRA_VDP
pushad
mov eax,%1
push eax
call print_dword
pop eax
popad
%endif
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

section .data align=64

	DECL TAB336

	%assign i 0
	%rep 240
		dd (i * 336)
	%assign i i+1
	%endrep

	ALIGN32

	DECL TAB320

	%assign i 0
	%rep 240
		dd (i * 336)
	%assign i i+1
	%endrep

	ALIGN32
	
	Mask_N	dd 0xFFFFFFFF, 0xFFF0FFFF, 0xFF00FFFF, 0xF000FFFF, 
			dd 0x0000FFFF, 0x0000FFF0, 0x0000FF00, 0x0000F000

	Mask_F	dd 0xFFFFFFFF, 0xFFFF0FFF, 0xFFFF00FF, 0xFFFF000F, 
			dd 0xFFFF0000, 0x0FFF0000, 0x00FF0000, 0x000F0000


section .bss align=64

	extern VRam
	extern VRam_Flag
	extern CRam
	extern CRam_Flag
	extern VSRam
	extern VDP_Reg
	extern VDP_Current_Line
	extern VDP_Status
	extern H_Cell
	extern H_Win_Mul
	extern H_Pix
	extern H_Pix_Begin
	extern H_Scroll_Mask
	extern H_Scroll_CMul
	extern H_Scroll_CMask
	extern V_Scroll_CMask
	extern V_Scroll_MMask
	extern Win_X_Pos
	extern Win_Y_Pos
	extern ScrA_Addr
	extern ScrB_Addr
	extern Win_Addr
	extern Spr_Addr
	extern H_Scroll_Addr

	struc vx
		.Mode		resd 1
		.State		resd 1
		.AF_Data	resd 1
		.AF_St		resd 1
		.AF_Len		resd 1
	endstruc

	resw (320 + 32)

	DECL MD_Screen
	resw (336 * 240)
	
	resw (320 + 32)

	DECL MD_Palette
	resw 0x100

	DECL Palette
	resw 0x1000

	DECL Sprite_Struct
	resd (0x100 * 8)
	
	DECL Sprite_Visible
	resd 0x100

	DECL Data_Spr
	.H_Min			resd 1
	.H_Max			resd 1

	ALIGN_32
	
	DECL Data_Misc
;	.Pattern_Adr	resd 1
	.Line_7			resd 1
	.X				resd 1
	.Cell			resd 1
	.Start_A		resd 1
	.Lenght_A		resd 1
	.Start_W		resd 1
	.Lenght_W		resd 1
	.Mask			resd 1
	.Spr_End		resd 1
	.Next_Cell		resd 1
	.Palette		resd 1
	.Borne			resd 1

	ALIGN_4
	
	DECL pattern_adr
	resd 1
	DECL Sprite_Over
	resd 1
	DECL Mode_555
	resd 1

section .text align=64


;****************************************

; macro GET_X_OFFSET
; param :
; %1 = 0 pour scroll B et 1 scroll A
; sortie :
; - esi contient l'offset X de la ligne en cour
; - edi contient le numero de ligne en cour

%macro GET_X_OFFSET 1

	mov eax, [VDP_Current_Line]
	mov ebx, [H_Scroll_Addr]			; ebx pointe sur les donnees du H-Scroll
	mov edi, eax
	and eax, [H_Scroll_Mask]

%if %1 > 0
	mov esi, [ebx + eax * 4]			; X Cell offset
%else
	mov si, [ebx + eax * 4 + 2]			; X Cell offset
%endif

%endmacro


;****************************************

; macro UPDATE_Y_OFFSET
; entr�e :
; eax = cell courant
; param :
; %1 = 0 pour scroll B et 1 pour scroll A
; %2 = 0 pour mode normal et 1 pour mode entrelac�
; sortie :
; edi = Offset y en fonction du cell courant

%macro UPDATE_Y_OFFSET 2

	mov eax, [Data_Misc.Cell]				; Cell courant pour le V Scroll
	test eax, 0xFF81						; en dehors des limites de la VRAM ? on change pas ...
	jnz short %%End
	mov edi, [VDP_Current_Line]				; edi = numero ligne

%if %1 > 0
	mov eax, [VSRam + eax * 2 + 0]
%else
	mov ax, [VSRam + eax * 2 + 2]
%endif

%if %2 > 0
	shr eax, 1								; on divise le Y scroll par 2 si on est en entrelac�
%endif

	add edi, eax
	mov eax, edi
	shr edi, 3								; V Cell Offset
	and eax, byte 7							; on ajuste pour un pattern
	and edi, [V_Scroll_CMask]				; on empeche V Cell Offset de deborder
	mov [Data_Misc.Line_7], eax

%%End

%endmacro


;****************************************

; macro GET_PATTERN_INFO
; entr�e :
; - H_Scroll_CMul doivent etre correctement initialis�
; - esi et edi contiennent offset X et offset Y respectivement
; param :
; %1 = 0 pour scroll B et 1 pour scroll A
; sortie :
; -  ax = Pattern Info

%macro GET_PATTERN_INFO 1

	mov cl, [H_Scroll_CMul]
	mov eax, edi								; eax = V Cell Offset
	mov edx, esi								; edx = H Cell Offset

	shl eax, cl									; eax = V Cell Offset * H Width

%if %1 > 0
	mov ebx, [ScrA_Addr]
%else
	mov ebx, [ScrB_Addr]
%endif

	add edx, eax								; edx = (V Offset / 8) * H Width + (H Offset / 8)
	mov ax, [ebx + edx * 2]						; ax = Cell Info inverse

%endmacro


;****************************************

; macro GET_PATTERN_DATA
; param :
; %1 = 0 pour mode normal et 1 pour mode entrelac�
; %2 = 0 pour les scrolls, 1 pour la window
; entr�e :
; - ax = Pattern Info
; - edi contient offset Y (si %2 = 0)
; - Data_Misc.Line_7 contient Line & 7 (si %2 != 0)
; sortie :
; - ebx = Pattern Data
; - edx = Num Palette * 16

%macro GET_PATTERN_DATA 2

	mov ebx, [Data_Misc.Line_7]					; ebx = V Offset
	mov edx, eax								; edx = Cell Info
	mov ecx, eax								; ecx = Cell Info
	shr edx, 9
	and ecx, 0x7FF
	and edx, byte 0x30							; edx = Palette

%if %1 > 0
	shl ecx, 6									; numero pattern * 64 (entrelac�)
%else
	shl ecx, 5									; numero pattern * 32 (normal)
%endif

	test eax, 0x1000							; on teste si V-Flip ?
	jz %%No_V_Flip								; si oui alors

	xor ebx, byte 7

%%No_V_Flip

%if %1 > 0
	mov ebx, [VRam + ecx + ebx * 8]				; ebx = Ligne du pattern = Data Pattern (entrelac�)
%else
	mov ebx, [VRam + ecx + ebx * 4]				; ebx = Ligne du pattern = Data Pattern (normal)
%endif

%endmacro


;****************************************

; macro MAKE_SPRITE_STRUCT
; param :
; %1 = 0 pour mode normal et 1 pour mode entrelac�

%macro MAKE_SPRITE_STRUCT 1

	mov ebp, [Spr_Addr]
	xor edi, edi							; edi = 0
	mov esi, ebp							; esi pointe sur la table de donnees sprites
	jmp short %%Loop

	ALIGN32
	
%%Loop
		mov ax, [ebp + 0]						; ax = Pos Y
		mov cx, [ebp + 6]						; cx = Pos X
		mov dl, [ebp + (2 ^ 1)]					; dl = Sprite Size
	%if %1 > 0
		shr eax, 1								; si entrelac�, la position est divis� par 2
	%endif
		mov dh, dl
		and eax, 0x1FF
		and ecx, 0x1FF
		and edx, 0x0C03							; on isole Size X et Size Y dans dh et dl respectivement
		sub eax, 0x80							; eax = Pos Y correct
		sub ecx, 0x80							; ecx = Pos X correct
		shr dh, 2								; dh = Size X - 1
		mov [Sprite_Struct + edi + 4], eax		; on stocke Pos Y
		inc dh									; dh = Size X
		mov [Sprite_Struct + edi + 0], ecx		; on stocke Pos X
		mov bl, dh								; bl = Size X
		mov [Sprite_Struct + edi + 8], dh		; on stocke Size X
		and ebx, byte 7							; ebx = Size X
		mov [Sprite_Struct + edi + 12], dl		; on stocke Size Y - 1
		and edx, byte 3							; edx = Size Y - 1
		lea ecx, [ecx + ebx * 8 - 1]			; ecx = Pos X Max
		lea eax, [eax + edx * 8 + 7]			; eax = Pos Y Max
		mov bl, [ebp + (3 ^ 1)]					; bl = Pointeur vers le prochain sprite
		mov [Sprite_Struct + edi + 16], ecx		; on stocke Pos X Max
		mov dx, [ebp + 4]						; dx = 1st tile du sprite
		mov [Sprite_Struct + edi + 20], eax		; on stocke Pos Y Max
		add edi, byte (8 * 4)					; on avance sur la prochaine structure sprite
		and ebx, byte 0x7F						; on enleve le bit de poid fort
;PRINT_BYTE bl
		mov [Sprite_Struct + edi - 32 + 24], dx	; on stocke le 1st tile du sprite
		jz short %%End							; si le prochain pointeur est 0 alors fini
		lea ebp, [esi + ebx * 8]				; ebp pointe sur le prochain sprite
		cmp edi, (8 * 4 * 80)					; si on a d�j� d�finit 80 sprites alors on arrete
		jb near %%Loop

%%End
	sub edi, 8 * 4
	mov [Data_Misc.Spr_End], edi			; on stocke le pointeur du dernier sprite

PRINT_SPRITE_STRUCT

%endmacro


;****************************************

; macro MAKE_SPRITE_STRUCT_PARTIAL
; param :

%macro MAKE_SPRITE_STRUCT_PARTIAL 0

	mov ebp, [Spr_Addr]
;	xor eax, eax
	xor ebx, ebx
	xor edi, edi							; edi = 0
	mov esi, ebp							; esi pointe sur la table de donnees sprites
	jmp short %%Loop

	ALIGN32
	
%%Loop
		mov al, [ebp + (2 ^ 1)]					; al = Sprite Size
		mov bl, [ebp + (3 ^ 1)]					; bl = Pointeur vers le prochain sprite
		mov cx, [ebp + 6]						; cx = Pos X
		mov dx, [ebp + 4]						; dx = 1st tile du sprite
		and ecx, 0x1FF
		mov [Sprite_Struct + edi + 24], dx		; on stocke le 1st tile du sprite
		sub ecx, 0x80							; ecx = Pos X correct
		and eax, 0x0C
		mov [Sprite_Struct + edi + 0], ecx		; on stocke Pos X
		lea ecx, [ecx + eax * 2 + 7]			; ecx = Pos X Max
		and bl, 0x7F							; on enleve le bit de poid fort
		mov [Sprite_Struct + edi + 16], ecx		; on stocke Pos X Max
		jz short %%End							; si le prochain pointeur est 0 alors fini

		add edi, byte (8 * 4)					; on avance sur la prochaine structure sprite
		lea ebp, [esi + ebx * 8]				; ebp pointe sur le prochain sprite
		cmp edi, (8 * 4 * 80)					; si on a d�j� d�finit 80 sprites alors on arrete
		jb short %%Loop

%%End

;PRINT_SPRITE_STRUCT

%endmacro


;****************************************

; macro UPDATE_MASK_SPRITE
; param :
; %1 = Sprite Limit Emulation (1 = enable et 0 = disable)
; entr�e :
; - Sprite_Struct doit etre correctement initialis�
; sortie :
; - edi pointe sur la premi�re structure sprite � afficher.
; - edx contient le numero de ligne

%macro UPDATE_MASK_SPRITE 1

;PRINT_TAG 100
;PRINT_MDSCREEN
;push eax
;mov eax,[Data_Misc.Spr_End]
;PRINT_DWORD eax
;pop eax
	xor edi, edi
%if %1 > 0
	mov ecx, [H_Cell]
%endif
	xor ax, ax						; used for masking
	mov ebx, [H_Pix]
	xor esi, esi
	mov edx, [VDP_Current_Line]
	jmp %%Loop_1

	ALIGN4
	
%%Loop_1
;PRINT_TAG 1
		cmp [Sprite_Struct + edi + 4], edx			; on teste si le sprite est sur la ligne courante
		jg %%Out_Line_1
		cmp [Sprite_Struct + edi + 20], edx			; on teste si le sprite est sur la ligne courante
		jl %%Out_Line_1

;PRINT_TAG 3
%if %1 > 0
		sub ecx, [Sprite_Struct + edi + 8]
%endif

;push eax
;mov eax,[Sprite_Struct + edi + 0]
;PRINT_DWORD eax
;PRINT_DWORD EBX
;mov eax,[Sprite_Struct + edi + 16]
;PRINT_DWORD eax
;pop eax

		cmp [Sprite_Struct + edi + 0], ebx			; on teste si le sprite n'est pas en dehors de l'�cran
		jge %%Out_Line_1_2
		cmp dword [Sprite_Struct + edi + 16], 0		; on teste si le sprite n'est pas en dehors de l'�cran
		jl %%Out_Line_1_2

;PRINT_TAG 4
		mov [Sprite_Visible + esi], edi
		add esi, byte 4

%%Out_Line_1_2
		add edi, byte (8 * 4)
		cmp edi, [Data_Misc.Spr_End]
		jle %%Loop_2

		jmp %%End

	ALIGN4

%%Out_Line_1
;PRINT_TAG 2
		add edi, byte (8 * 4)
		cmp edi, [Data_Misc.Spr_End]
		jle %%Loop_1

		jmp %%End

	ALIGN4

%%Loop_2
;PRINT_TAG 5
		cmp [Sprite_Struct + edi + 4], edx			; on teste si le sprite est sur la ligne courante
		jg %%Out_Line_2
		cmp [Sprite_Struct + edi + 20], edx			; on teste si le sprite est sur la ligne courante
		jl %%Out_Line_2

%%Loop_2_First
;PRINT_TAG 6
		cmp dword [Sprite_Struct + edi + 0], -128	; le sprite est-il un mask ?
		je %%End								; next sprites are masked

%if %1 > 0
		sub ecx, [Sprite_Struct + edi + 8]
%endif
		cmp [Sprite_Struct + edi + 0], ebx			; on teste si le sprite n'est pas en dehors de l'�cran
		jge %%Out_Line_2
		cmp dword [Sprite_Struct + edi + 16], 0		; on teste si le sprite n'est pas en dehors de l'�cran
		jl %%Out_Line_2

;PRINT_TAG 7
		mov [Sprite_Visible + esi], edi
		add esi, byte 4

%%Out_Line_2
;PRINT_TAG 8
		add edi, byte (8 * 4)

;PRINT_DWORD edi
%if %1 > 0
		cmp ecx, byte 0
		jle %%Sprite_Overflow
%endif
		cmp edi, [Data_Misc.Spr_End]
		jle %%Loop_2
		jmp %%End

	ALIGN4
	
%%Sprite_Overflow
;PRINT_TAG 9
	cmp edi, [Data_Misc.Spr_End]
	jg %%End
	jmp %%Loop_3

	ALIGN4
	
	%%Loop_3
		cmp [Sprite_Struct + edi + 4], edx			; on teste si le sprite est sur la ligne courante
		jg %%Out_Line_3
		cmp [Sprite_Struct + edi + 20], edx			; on teste si le sprite est sur la ligne courante
		jl %%Out_Line_3

		or byte [VDP_Status], 0x40
		jmp %%End

%%Out_Line_3
		add edi, byte (8 * 4)
		cmp edi, [Data_Misc.Spr_End]
		jle %%Loop_3
		jmp %%End

	ALIGN4

%%End
;PRINT_TAG 10
	mov [Data_Misc.Borne], esi
PRINT_SPRITE_VISIBLE
;PRINT_MDSCREEN
%endmacro


;****************************************

; macro PUTPIXEL_P0
; param :
; %1 = Num�ro du pixel
; %2 = Mask pour isoler le bon pixel
; %3 = D�calage
; %4 = 0 pour scroll B et 1 sinon
; %5 = Shadow/Highlight enable
; entr�e :
; - ebx = Pattern Data
; - edx = Num�ro de palette * 64

%macro PUTPIXEL_P0 5

	mov eax, ebx
	and eax, %2
	jz short %%Trans

%if %4 > 0
	%if %5 > 0
		mov cl, [MD_Screen + ebp * 2 + (%1 * 2) + 1]
		test cl, PRIO_B
		jnz short %%Trans
	%else
		test byte [MD_Screen + ebp * 2 + (%1 * 2) + 1], PRIO_B
		jnz short %%Trans
	%endif
%endif

%if %3 > 0
	shr eax, %3
%endif

%if %4 > 0
	%if %5 > 0
		and cl, SHAD_B
		add al, dl
		add al, cl
	%else
		add al, dl
	%endif
%else
	%if %5 > 0
		lea eax, [eax + edx + SHAD_W]
	%else
		add al, dl
	%endif
%endif

;PRINT_PIXEL8 %1

	mov [MD_Screen + ebp * 2 + (%1 * 2)], al	; on affiche le pixel

%%Trans

%endmacro


;****************************************

; macro PUTPIXEL_P1
; param :
; %1 = Num�ro du pixel
; %2 = Mask pour isoler le bon pixel
; %3 = D�calage
; %4 = Shadow/Highlight enable
; entr�e :
; - ebx = Pattern Data
; - edx = Num�ro de palette * 64

%macro PUTPIXEL_P1 4

	mov eax, ebx
	and eax, %2
	jz short %%Trans

%if %3 > 0
	shr eax, %3
%endif

	lea eax, [eax + edx + PRIO_W]
	mov [MD_Screen + ebp * 2 + (%1 * 2)], ax

%%Trans

%endmacro


;****************************************

; macro PUTPIXEL_SPRITE
; param :
; %1 = Num�ro du pixel
; %2 = Mask pour isoler le bon pixel
; %3 = D�calage
; %4 = Priorit�
; %5 = Highlight/Shadow Enable
; entr�e :
; - ebx = Pattern Data
; - edx = Num�ro de palette * 16

%macro PUTPIXEL_SPRITE 5

PRINT_PUTPIXEL_SPRITE

;PRINT_TAG 1
	mov eax, ebx
	and eax, %2
	jz %%TransEmpty

;PRINT_TAG 2
	mov cl, [MD_Screen + ebp * 2 + (%1 * 2) + 16 + 1]
;PRINT_DWORD ebp
;PRINT_DWORD %1
;PRINT_BYTE cl
;PRINT_DWORD %4

	test cl, (PRIO_B + SPR_B - %4)
	jz %%Affich

;PRINT_TAG 3
%%Prio
	or ch, cl
%if %4 < 1
	or byte [MD_Screen + ebp * 2 + (%1 * 2) + 16 + 1], SPR_B
%endif
	jmp %%Trans

ALIGN4

%%Affich
;PRINT_TAG 4

%if %3 > 0
	shr eax, %3
%endif

PRINT_PIXEL_VALUE

	lea eax, [eax + edx + SPR_W]


%if %5 > 0
;PRINT_TAG 5
	%if %4 < 1
;PRINT_TAG 6
		and cl, SHAD_B | HIGH_B
	%else
;PRINT_TAG 7
		and cl, HIGH_B
	%endif

	cmp eax, (0x3E + SPR_W)
	jb %%Normal
	ja %%Shadow

%%Highlight
;PRINT_TAG 10
	or word [MD_Screen + ebp * 2 + (%1 * 2) + 16], HIGH_W
	jmp %%Trans
	
%%Shadow
;PRINT_TAG 9
	or word [MD_Screen + ebp * 2 + (%1 * 2) + 16], SHAD_W
	jmp %%Trans

%%Normal
;PRINT_TAG 8
	add al, cl

%endif

;PRINT_TAG 11
	mov [MD_Screen + ebp * 2 + (%1 * 2) + 16], ax

%%Trans
;PRINT_TAG 12
%%TransEmpty

%endmacro


;****************************************

; macro PUTLINE_P0
; param :
; %1 = 0 pour scroll B et 1 sinon
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp pointe sur dest

%macro PUTLINE_P0 2

%if %1 < 1
	%if %2 > 0
		mov dword [MD_Screen + ebp * 2 +  0], SHAD_D
		mov dword [MD_Screen + ebp * 2 +  4], SHAD_D
		mov dword [MD_Screen + ebp * 2 +  8], SHAD_D
		mov dword [MD_Screen + ebp * 2 + 12], SHAD_D
	%else
		mov dword [MD_Screen + ebp * 2 +  0], 0x00000000
		mov dword [MD_Screen + ebp * 2 +  4], 0x00000000
		mov dword [MD_Screen + ebp * 2 +  8], 0x00000000
		mov dword [MD_Screen + ebp * 2 + 12], 0x00000000
	%endif
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P0 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_P0 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_P0 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_P0 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_P0 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_P0 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_P0 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_P0 7, 0x000f0000, 16, %1, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P0
; param :
; %1 = 0 pour scroll B et 1 sinon
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp pointe sur dest

%macro PUTLINE_FLIP_P0 2

%if %1 < 1
	%if %2 > 0
		mov dword [MD_Screen + ebp * 2 +  0], SHAD_D
		mov dword [MD_Screen + ebp * 2 +  4], SHAD_D
		mov dword [MD_Screen + ebp * 2 +  8], SHAD_D
		mov dword [MD_Screen + ebp * 2 + 12], SHAD_D
	%else
		mov dword [MD_Screen + ebp * 2 +  0], 0x00000000
		mov dword [MD_Screen + ebp * 2 +  4], 0x00000000
		mov dword [MD_Screen + ebp * 2 +  8], 0x00000000
		mov dword [MD_Screen + ebp * 2 + 12], 0x00000000
	%endif
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P0 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_P0 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_P0 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_P0 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_P0 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_P0 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_P0 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_P0 7, 0x0000f000, 12, %1, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_P1
; %1 = 0 pour scroll B et 1 sinon
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp pointe sur dest

%macro PUTLINE_P1 2

%if %1 < 1
	mov dword [MD_Screen + ebp * 2 +  0], 0x00000000
	mov dword [MD_Screen + ebp * 2 +  4], 0x00000000
	mov dword [MD_Screen + ebp * 2 +  8], 0x00000000
	mov dword [MD_Screen + ebp * 2 + 12], 0x00000000
%else
	%if %2 > 0

		; Faster on almost CPU (because of pairable instructions)

		mov eax, [MD_Screen + ebp * 2 +  0]
		mov ecx, [MD_Screen + ebp * 2 +  4]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [MD_Screen + ebp * 2 +  0], eax
		mov [MD_Screen + ebp * 2 +  4], ecx
		mov eax, [MD_Screen + ebp * 2 +  8]
		mov ecx, [MD_Screen + ebp * 2 + 12]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [MD_Screen + ebp * 2 +  8], eax
		mov [MD_Screen + ebp * 2 + 12], ecx

		; Faster on K6 CPU

		;and dword [MD_Screen + ebp * 2 +  0], NOSHAD_D
		;and dword [MD_Screen + ebp * 2 +  4], NOSHAD_D
		;and dword [MD_Screen + ebp * 2 +  8], NOSHAD_D
		;and dword [MD_Screen + ebp * 2 + 12], NOSHAD_D
	%endif
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P1 0, 0x0000f000, 12, %2
	PUTPIXEL_P1 1, 0x00000f00,  8, %2
	PUTPIXEL_P1 2, 0x000000f0,  4, %2
	PUTPIXEL_P1 3, 0x0000000f,  0, %2
	PUTPIXEL_P1 4, 0xf0000000, 28, %2
	PUTPIXEL_P1 5, 0x0f000000, 24, %2
	PUTPIXEL_P1 6, 0x00f00000, 20, %2
	PUTPIXEL_P1 7, 0x000f0000, 16, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P1
; %1 = 0 pour scroll B et 1 sinon
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp pointe sur dest

%macro PUTLINE_FLIP_P1 2

%if %1 < 1
	mov dword [MD_Screen + ebp * 2 +  0], 0x00000000
	mov dword [MD_Screen + ebp * 2 +  4], 0x00000000
	mov dword [MD_Screen + ebp * 2 +  8], 0x00000000
	mov dword [MD_Screen + ebp * 2 + 12], 0x00000000
%else
	%if %2 > 0

		; Faster on almost CPU (because of pairable instructions)

		mov eax, [MD_Screen + ebp * 2 +  0]
		mov ecx, [MD_Screen + ebp * 2 +  4]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [MD_Screen + ebp * 2 +  0], eax
		mov [MD_Screen + ebp * 2 +  4], ecx
		mov eax, [MD_Screen + ebp * 2 +  8]
		mov ecx, [MD_Screen + ebp * 2 + 12]
		and eax, NOSHAD_D
		and ecx, NOSHAD_D
		mov [MD_Screen + ebp * 2 +  8], eax
		mov [MD_Screen + ebp * 2 + 12], ecx

		; Faster on K6 CPU

		;and dword [MD_Screen + ebp * 2 +  0], NOSHAD_D
		;and dword [MD_Screen + ebp * 2 +  4], NOSHAD_D
		;and dword [MD_Screen + ebp * 2 +  8], NOSHAD_D
		;and dword [MD_Screen + ebp * 2 + 12], NOSHAD_D
	%endif
%endif

	test ebx, ebx
	jz near %%Full_Trans

	PUTPIXEL_P1 0, 0x000f0000, 16, %2
	PUTPIXEL_P1 1, 0x00f00000, 20, %2
	PUTPIXEL_P1 2, 0x0f000000, 24, %2
	PUTPIXEL_P1 3, 0xf0000000, 28, %2
	PUTPIXEL_P1 4, 0x0000000f,  0, %2
	PUTPIXEL_P1 5, 0x000000f0,  4, %2
	PUTPIXEL_P1 6, 0x00000f00,  8, %2
	PUTPIXEL_P1 7, 0x0000f000, 12, %2

%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_SPRITE
; param :
; %1 = Priorit�
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp pointe sur dest mais sans le screen

%macro PUTLINE_SPRITE 2

PRINT_DEBUG_PUTLINE_SPRITE

	xor ecx, ecx
	add ebp, [esp]

	PUTPIXEL_SPRITE 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_SPRITE 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 7, 0x000f0000, 16, %1, %2

	and ch, 0x20
	sub ebp, [esp]
	or byte [VDP_Status], ch

%endmacro


;****************************************

; macro PUTLINE_SPRITE_FLIP
; param :
; %1 = Priorit�
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp pointe sur dest

%macro PUTLINE_SPRITE_FLIP 2

PRINT_DEBUG_PUTLINE_SPRITE

	xor ecx, ecx
	add ebp, [esp]

	PUTPIXEL_SPRITE 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_SPRITE 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 7, 0x0000f000, 12, %1, %2

	and ch, 0x20
	sub ebp, [esp]
	or byte [VDP_Status], ch

%endmacro

	
;****************************************

; macro UPDATE_PALETTE
; param :
; %1 = Highlight/Shadow Enable

%macro UPDATE_PALETTE 1

	xor eax, eax
	mov byte [CRam_Flag], 0						; on update la palette, on remet le flag a 0 pour modified
	mov cx, 0x7BEF
	xor edx, edx
	test byte [Mode_555], 1
	mov ebx, (64 / 2) - 1								; ebx = Nombre de couleurs
	jz short %%Loop

	mov cx, 0x3DEF
	jmp short %%Loop

	ALIGN32

%%Loop
		mov ax, [CRam + ebx * 4 + 0]					; ax = data color
		mov dx, [CRam + ebx * 4 + 2]					; dx = data color
		and ax, 0x0FFF
		and dx, 0x0FFF
		mov ax, [Palette + eax * 2]
		mov dx, [Palette + edx * 2]
		mov [MD_Palette + ebx * 4 + 0], ax				; couleur normal
		mov [MD_Palette + ebx * 4 + 2], dx				; couleur normal

%if %1 > 0
		mov [MD_Palette + ebx * 4 + 192 * 2 + 0], ax	; couleur normal
		mov [MD_Palette + ebx * 4 + 192 * 2 + 2], dx	; couleur normal
		shr ax, 1
		shr dx, 1
		and ax, cx										; on divise par 2 les composantes pour le shadow
		and dx, cx										; on divise par 2 les composantes pour le shadow
		mov [MD_Palette + ebx * 4 + 64 * 2 + 0], ax		; couleur sombre
		mov [MD_Palette + ebx * 4 + 64 * 2 + 2], dx		; couleur sombre
		add ax, cx
		add dx, cx
		mov [MD_Palette + ebx * 4 + 128 * 2 + 0], ax	; couleur clair
		mov [MD_Palette + ebx * 4 + 128 * 2 + 2], dx	; couleur clair
%endif

		dec ebx										; si on a pas encore fait les 64 couleurs
		jns short %%Loop							; alors on continue

		mov ebx, [VDP_Reg + 7 * 4]
		and ebx, byte 0x3F
		mov ax, [MD_Palette + ebx * 2]
		mov [MD_Palette + 0 * 2], ax

%if %1 > 0
		mov [MD_Palette + 0 * 2 + 192 * 2], ax
		shr ax, 1
		and ax, cx
		mov [MD_Palette + 0 * 2 + 64 * 2], ax
		add ax, cx
		mov [MD_Palette + 0 * 2 + 128 * 2], ax
%endif

%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_B
; param :
; %1 = 1 pour mode entrelac� et 0 pour mode normal
; %2 = 1 si V-Scroll mode en 2 cell et 0 si full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_B 3

	mov ebp, [esp]				; ebp pointe sur la surface o� l'on rend

	GET_X_OFFSET 0

PRINT_XOFFSET

	mov eax, esi				; eax = scroll X inv
	xor esi, 0x3FF				; esi = scroll X norm
	and eax, byte 7				; eax = completion pour offset
	shr esi, 3					; esi = cell courant
	add ebp, eax				; ebp mis � jour pour clipping
	mov ebx, esi
	and esi, [H_Scroll_CMask]	; on empeche H Cell Offset de deborder
	and ebx, byte 1
	mov eax, [H_Cell]
	sub ebx, byte 2				; on d�marre au cell -2 ou -1 (pour le V Scroll)
	mov [Data_Misc.X], eax		; nombre de cell � afficher
	mov [Data_Misc.Cell], ebx	; Cell courant pour le V Scroll


	mov edi, [VDP_Current_Line]		; edi = numero ligne
	mov eax, [VSRam + 2]

%if %1 > 0
	shr eax, 1						; on divise le Y scroll par 2 si on est en entrelac�
%endif

	add edi, eax
	mov eax, edi
	shr edi, 3								; V Cell Offset
	and eax, byte 7							; on ajuste pour un pattern
	and edi, [V_Scroll_CMask]				; on empeche V Cell Offset de deborder

;PRINT_YOFFSET

	mov [Data_Misc.Line_7], eax

;PRINT_MDSCREEN
	jmp short %%First_Loop

	ALIGN32

	%%Loop

;PRINT_MDSCREEN
%if %2 > 0
		UPDATE_Y_OFFSET 0, %1
%endif

	%%First_Loop

PRINT_YOFFSET

		GET_PATTERN_INFO 0

;PRINT_PATTERN_INFO

		GET_PATTERN_DATA %1, 0

;PRINT_PATTERN_DATA
;PRINT_PUTLINE
		
		test eax, 0x0800							; on teste si H-Flip ?
		jz near %%No_H_Flip							; si oui alors
;PRINT_TAG 1

	%%H_Flip

			test eax, 0x8000						; on teste le priorite du pattern courant
			jnz near %%H_Flip_P1

	%%H_Flip_P0
;PRINT_TAG 3
				PUTLINE_FLIP_P0 0, %3
				jmp %%End_Loop

	ALIGN32

	%%H_Flip_P1
;PRINT_TAG 2
				PUTLINE_FLIP_P1 0, %3
				jmp %%End_Loop

	ALIGN32
	
	%%No_H_Flip
;PRINT_TAG 4

			test eax, 0x8000						; on teste le priorite du pattern courant
			jnz near %%No_H_Flip_P1

	%%No_H_Flip_P0
;PRINT_TAG 6
				PUTLINE_P0 0, %3
				jmp %%End_Loop

	ALIGN32

	%%No_H_Flip_P1
;PRINT_TAG 5
				PUTLINE_P1 0, %3
				jmp short %%End_Loop

	ALIGN32

	%%End_Loop
;PRINT_TAG 7
		inc dword [Data_Misc.Cell]		; Prochain H cell pour le V Scroll
		inc esi							; Prochain H cell
		add ebp, byte 8					; on avance sur le prochain pattern
		and esi, [H_Scroll_CMask]		; on empeche H Offset de deborder
		dec byte [Data_Misc.X]			; un cell de moins � traiter
		jns near %%Loop
		
%%End
;PRINT_MDSCREEN


%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_A_WIN
; param :
; %1 = 1 pour mode entrelac� et 0 pour mode normal
; %2 = 1 si V-Scroll mode en 2 cell et 0 si full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_A_WIN 3

	mov eax, [VDP_Current_Line]
	mov cl, [VDP_Reg + 18 * 4]
	shr eax, 3
	mov ebx, [H_Cell]
	shr cl, 7							; cl = 1 si window at bottom
;PRINT_TAG 1
	cmp eax, [Win_Y_Pos]
	setae ch							; ch = 1 si current line >= pos Y window
	xor cl, ch							; cl = 0 si line window sinon line Scroll A
	jz near %%Full_Win

;PRINT_TAG 2 
	test byte [VDP_Reg + 17 * 4], 0x80
	mov edx, [Win_X_Pos]
	jz short %%Win_Left

%%Win_Right
	sub ebx, edx
	mov [Data_Misc.Start_W], edx		; Start Win (Cell)
	mov [Data_Misc.Lenght_W], ebx		; Lenght Win (Cell)
	dec edx								; 1 cell en moins car on affiche toujours le dernier � part
	mov dword [Data_Misc.Start_A], 0	; Start Scroll A (Cell)
	mov [Data_Misc.Lenght_A], edx		; Lenght Scroll A (Cell)

PRINT_TMP_VALUE_WIN
test edx,edx

	jns short %%Scroll_A
	jmp %%Window

	ALIGN4
	
%%Win_Left
	sub ebx, edx
	mov dword [Data_Misc.Start_W], 0	; Start Win (Cell)
	mov [Data_Misc.Lenght_W], edx		; Lenght Win (Cell)
	dec ebx								; 1 cell en moins car on affiche toujours le dernier � part
	mov [Data_Misc.Start_A], edx		; Start Scroll A (Cell)
	mov [Data_Misc.Lenght_A], ebx		; Lenght Scroll A (Cell)

PRINT_TMP_VALUE_WIN
test ebx,ebx
	jns short %%Scroll_A
	jmp %%Window

	ALIGN4

%%Scroll_A


	mov ebp, [esp]					; ebp pointe sur la surface o� l'on rend

	GET_X_OFFSET 1

	mov eax, esi					; eax = scroll X inv
	mov ebx, [Data_Misc.Start_A]	; Premier Cell
	xor esi, 0x3FF					; esi = scroll X norm
	and eax, byte 7					; eax = completion pour offset
	shr esi, 3						; esi = cell courant (d�but scroll A)
	mov [Data_Misc.Mask], eax		; mask pour le dernier pattern
	mov ecx, esi					; ecx = cell courant (d�but scroll A) 
	add esi, ebx					; esi = cell courant ajust� pour window clip
	and ecx, byte 1
	lea eax, [eax + ebx * 8]		; clipping + window clip
	sub ecx, byte 2					; on d�marre au cell -2 ou -1 (pour le V Scroll)
	and esi, [H_Scroll_CMask]		; on empeche H Cell Offset de deborder
	add ebp, eax					; ebp mis � jour pour clipping + window clip
	add ebx, ecx					; ebx = Cell courant pour le V Scroll

	mov edi, [VDP_Current_Line]		; edi = numero ligne
	mov [Data_Misc.Cell], ebx		; Cell courant pour le V Scroll

	jns short %%Not_First_Cell

	mov eax, [VSRam + 0]
	jmp short %%First_VScroll_OK

%%Not_First_Cell
	and ebx, [V_Scroll_MMask]
	mov eax, [VSRam + ebx * 2]

%%First_VScroll_OK



%if %1 > 0
	shr eax, 1						; on divise le Y scroll par 2 si on est en entrelac�
%endif

	add edi, eax
	mov eax, edi
	shr edi, 3								; V Cell Offset
	and eax, byte 7							; on ajuste pour un pattern
	and edi, [V_Scroll_CMask]				; on empeche V Cell Offset de deborder
	mov [Data_Misc.Line_7], eax


PRINT_DEBUG_SCROLL_A

	jmp short %%First_Loop_SCA

	ALIGN32

%%Loop_SCA

PRINT_DEBUG_SCROLL_A


%if %2 > 0
		UPDATE_Y_OFFSET 1, %1

%endif

%%First_Loop_SCA


PRINT_OFFSETS

		GET_PATTERN_INFO 1

PRINT_PATTERN_INFO

		GET_PATTERN_DATA %1, 0

PRINT_PATTERN_DATA
	
		test eax, 0x0800							; on teste si H-Flip ?
		jz near %%No_H_Flip							; si oui alors

	%%H_Flip
			test eax, 0x8000						; on teste le priorite du pattern courant
			jnz near %%H_Flip_P1

	%%H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp %%End_Loop

	ALIGN32

	%%H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp %%End_Loop

	ALIGN32
	
	%%No_H_Flip
			test eax, 0x8000						; on teste le priorite du pattern courant
			jnz near %%No_H_Flip_P1

	%%No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp %%End_Loop

	ALIGN32

	%%No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp short %%End_Loop

	ALIGN32

	%%End_Loop
		inc dword [Data_Misc.Cell]		; Prochain H cell pour le V Scroll
		inc esi							; Prochain H cell
		add ebp, byte 8					; on avance sur le prochain pattern
		and esi, [H_Scroll_CMask]		; on empeche H Offset de deborder
		dec byte [Data_Misc.Lenght_A]	; un cell de moins � traiter pour Scroll A
		jns near %%Loop_SCA




%%LC_SCA

%if %2 > 0
	UPDATE_Y_OFFSET 1, %1
%endif

	GET_PATTERN_INFO 1
	GET_PATTERN_DATA %1, 0

	test eax, 0x0800							; on teste si H-Flip ?
	mov ecx, [Data_Misc.Mask]
	jz near %%LC_SCA_No_H_Flip					; si oui alors

	%%LC_SCA_H_Flip
		and ebx, [Mask_F + ecx * 4]				; on applique le mask
		test eax, 0x8000						; on teste le priorite du pattern courant
		jnz near %%LC_SCA_H_Flip_P1

	%%LC_SCA_H_Flip_P0
			PUTLINE_FLIP_P0 1, %3
			jmp %%LC_SCA_End

	ALIGN32

	%%LC_SCA_H_Flip_P1
			PUTLINE_FLIP_P1 1, %3
			jmp %%LC_SCA_End

	ALIGN32
	
	%%LC_SCA_No_H_Flip
		and ebx, [Mask_N + ecx * 4]				; on applique le mask
		test eax, 0x8000						; on teste le priorite du pattern courant
		jnz near %%LC_SCA_No_H_Flip_P1

	%%LC_SCA_No_H_Flip_P0
			PUTLINE_P0 1, %3
			jmp %%LC_SCA_End

	ALIGN32

	%%LC_SCA_No_H_Flip_P1
			PUTLINE_P1 1, %3
			jmp short %%LC_SCA_End

	ALIGN32

%%LC_SCA_End
	test byte [Data_Misc.Lenght_W], 0xFF
	jnz short %%Window
	jmp %%End





	ALIGN4

%%Full_Win
;PRINT_TAG 3
	xor esi, esi							; Start Win (Cell)
	mov edi, ebx							; Lenght Win (Cell)
	jmp short %%Window_Initialised

	ALIGN4

%%Window
;PRINT_TAG 4
	mov esi, [Data_Misc.Start_W]
	mov edi, [Data_Misc.Lenght_W]			; edi = Nb cell � rendre

%%Window_Initialised
	mov edx, [VDP_Current_Line]
	mov cl, [H_Win_Mul]
	mov ebx, edx							; ebx = Line
	mov ebp, [esp]							; ebp pointe sur la surface o� l'on rend
	shr edx, 3								; edx = Line / 8
	mov eax, [Win_Addr]
	shl edx, cl
	lea ebp, [ebp + esi * 8 + 8]			; pas de clipping pour la window, on rend directement le premier pixel
	lea eax, [eax + edx * 2]				; eax pointe sur les donnees des pattern pour la window
	and ebx, byte 7							; ebx = Line & 7 pour le V Flip
	mov [pattern_adr], eax		; on stocke ce pointeur
	mov [Data_Misc.Line_7], ebx				; on stocke Line & 7
	jmp short %%Loop_Win

	ALIGN32

%%Loop_Win
;PRINT_TAG 1001
		mov ebx, [pattern_adr]
;PRINT_DWORD esi
		mov ax, [ebx + esi * 2]
;PRINT_WORD ax

		GET_PATTERN_DATA %1, 1
;PRINT_PATTERN_DATA

		test ax, 0x0800					; on teste si H-Flip ?
		jz near %%W_No_H_Flip			; si oui alors

	%%W_H_Flip
;PRINT_TAG 1002
			test ax, 0x8000						; on teste le priorite du pattern courant
			jnz near %%W_H_Flip_P1

	%%W_H_Flip_P0
;PRINT_TAG 1004
				PUTLINE_FLIP_P0 1, %3
				jmp %%End_Loop_Win

	ALIGN32

	%%W_H_Flip_P1
;PRINT_TAG 1003
				PUTLINE_FLIP_P1 1, %3
				jmp %%End_Loop_Win

	ALIGN32
	
	%%W_No_H_Flip
;PRINT_TAG 1005
			test ax, 0x8000						; on teste le priorite du pattern courant
			jnz near %%W_No_H_Flip_P1

	%%W_No_H_Flip_P0
;PRINT_TAG 1007
				PUTLINE_P0 1, %3
				jmp %%End_Loop_Win

	ALIGN32

	%%W_No_H_Flip_P1
;PRINT_TAG 1006
				PUTLINE_P1 1, %3
				jmp short %%End_Loop_Win

	ALIGN32

	%%End_Loop_Win
;PRINT_TAG 1008
		inc esi						; prochain pattern		
		add ebp, byte 8				;    "        "   pour le rendu
		dec edi
		jnz near %%Loop_Win

%%End


%endmacro


;****************************************

; macro RENDER_LINE_SPR
; param :
; %1 = 1 pour mode entrelac� et 0 pour mode normal
; %2 = Shadow / Highlight (0 = Disable et 1 = Enable)

%macro RENDER_LINE_SPR 2

	test dword [Sprite_Over], 1
	jz near %%No_Sprite_Over

%%Sprite_Over

	UPDATE_MASK_SPRITE 1		; edi pointe sur le sprite � afficher
	xor edi, edi
	test esi, esi
	mov dword [Data_Misc.X], edi
	jnz near %%First_Loop
	jmp %%End					; on quitte

%%No_Sprite_Over
	UPDATE_MASK_SPRITE 0		; edi pointe sur le sprite � afficher
	xor edi, edi
	test esi, esi
	mov dword [Data_Misc.X], edi
	jnz short %%First_Loop
	jmp %%End					; on quitte

	ALIGN32

%%Sprite_Loop
		mov edx, [VDP_Current_Line]
%%First_Loop
;PRINT_TAG 1
		mov edi, [Sprite_Visible + edi]
		mov eax, [Sprite_Struct + edi + 24]		; eax = CellInfo du sprite

PRINT_CELL_INFO

		sub edx, [Sprite_Struct + edi + 4]		; edx = Line - Y Pos (Y Offset)
		mov ebx, eax							; ebx = CellInfo
		mov esi, eax							; esi = CellInfo

		shr bx, 9								; on va isoler la palette dans ebx
		mov ecx, edx							; ecx = Y Offset
		and ebx, 0x30							; on garde que le numero de palette * 16
	
		and esi, 0x7FF							; esi = numero du premier pattern du sprite
		mov [Data_Misc.Palette], ebx			; on stocke le numero de palette * 64 dans Palette
		and edx, 0xF8							; on efface les 3 bits de poids faibles = Num Pattern * 8
		mov ebx, [Sprite_Struct + edi + 12]		; ebx = Size Y
		and ecx, byte 7							; ecx = (Y Offset & 7) = Line du pattern courant
%if %1 > 0
		shl ebx, 6								; ebx = Size Y * 64
		lea edx, [edx * 8]						; edx = Num Pattern * 64
		shl esi, 6								; esi = pointe sur sur le contenu du pattern
%else
		shl ebx, 5								; ebx = Size Y * 32
		lea edx, [edx * 4]						; edx = Num Pattern * 32
		shl esi, 5								; esi = pointe sur sur le contenu du pattern
%endif

PRINT_FIRST_PAT

		test eax, 0x1000						; on teste pour V Flip
		jz %%No_V_Flip

	%%V_Flip
;PRINT_TAG 2
		sub ebx, edx
		xor ecx, 7								; ecx = 7 - (Y Offset & 7)
		add esi, ebx							; esi pointe sur le pattern � afficher
%if %1 > 0
		lea ebx, [ebx + edx + 64]				; on restore la valeur de ebx + 64
		lea esi, [esi + ecx * 8]				; et ainsi que sur la bonne ligne du pattern
		jmp short %%Suite
%else
		lea ebx, [ebx + edx + 32]				; on restore la valeur de ebx + 32
		lea esi, [esi + ecx * 4]				; et ainsi que sur la bonne ligne du pattern
		jmp short %%Suite
%endif

	ALIGN4
	
	%%No_V_Flip
;PRINT_TAG 3
		add esi, edx							; esi pointe sur le pattern � afficher
%if %1 > 0
		add ebx, byte 64						; on additionne 64 � ebx
		lea esi, [esi + ecx * 8]				; et ainsi que sur la bonne ligne du pattern
%else			
		add ebx, byte 32						; on additionne 32 � ebx
		lea esi, [esi + ecx * 4]				; et ainsi que sur la bonne ligne du pattern
%endif

	%%Suite
		mov [Data_Misc.Next_Cell], ebx			; la prochain Cell X de ce sprite se trouve � ebx octets
		mov edx, [Data_Misc.Palette]			; edx = Numero de palette * 64

		test eax, 0x800							; on teste H Flip
		jz near %%No_H_Flip
			
	%%H_Flip
;PRINT_TAG 4
		mov ebx, [Sprite_Struct + edi + 0]
		mov ebp, [Sprite_Struct + edi + 16]		; on positionne pour X
		cmp ebx, -7								; on teste pour la borne min du spr
		mov edi, [Data_Misc.Next_Cell]
		jg short %%Spr_X_Min_Norm
		mov ebx, -7								; borne min = clip ecran

	%%Spr_X_Min_Norm
		mov [Data_Spr.H_Min], ebx				; borne min = spr min

	%%Spr_X_Min_OK
		sub ebp, byte 7							; pour afficher le dernier pattern en premier
		jmp short %%Spr_Test_X_Max

	ALIGN4

	%%Spr_Test_X_Max_Loop
			sub ebp, byte 8							; on recule sur le pattern precedent (ecran)
			add esi, edi							; on va sur le prochain pattern (mem)

	%%Spr_Test_X_Max
			cmp ebp, [H_Pix]
			jge %%Spr_Test_X_Max_Loop

		test eax, 0x8000						; on teste la priorit�
		jnz near %%H_Flip_P1
		jmp short %%H_Flip_P0

	ALIGN32
	
	%%H_Flip_P0
	%%H_Flip_P0_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
			PUTLINE_SPRITE_FLIP 0, %2				; on affiche la ligne du pattern sprite

			sub ebp, byte 8							; on affiche le pattern precedent
			add esi, edi							; on va sur le prochain pattern
			cmp ebp, [Data_Spr.H_Min]				; on teste si on a fait tout les patterns du sprite
			jge near %%H_Flip_P0_Loop				; sinon on continue
		jmp %%End_Sprite_Loop

	ALIGN32
	
	%%H_Flip_P1
	%%H_Flip_P1_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
			PUTLINE_SPRITE_FLIP 1, %2				; on affiche la ligne du pattern sprite

			sub ebp, byte 8							; on affiche le pattern precedent
			add esi, edi							; on va sur le prochain pattern
			cmp ebp, [Data_Spr.H_Min]				; on teste si on a fait tout les patterns du sprite
			jge near %%H_Flip_P1_Loop				; sinon on continue
		jmp %%End_Sprite_Loop
				
	ALIGN32
	
	%%No_H_Flip
;PRINT_TAG 5
		mov ebx, [Sprite_Struct + edi + 16]
		mov ecx, [H_Pix]
		mov ebp, [Sprite_Struct + edi + 0]		; on position le pointeur ebp
		mov edi, [Data_Misc.Next_Cell]
PRINT_SPECIAL_DEBUG_LINE_SPR
		cmp ebx, ecx							; on teste pour la borne max du spr

		jl %%Spr_X_Max_Norm
		mov [Data_Spr.H_Max], ecx				; borne max = clip ecran
		jmp short %%Spr_Test_X_Min

	ALIGN4

	%%Spr_X_Max_Norm
		mov [Data_Spr.H_Max], ebx				; borne max = spr max
		jmp short %%Spr_Test_X_Min

	ALIGN4

	%%Spr_Test_X_Min_Loop
			add ebp, byte 8						; on avance sur le prochain pattern (ecran)
			add esi, edi						; on va sur le prochain pattern (mem)

	%%Spr_Test_X_Min
			cmp ebp, -7
			jl %%Spr_Test_X_Min_Loop

;PRINT_TAG 6
		test ax, 0x8000							; on teste la priorit�
		jnz near %%No_H_Flip_P1
		jmp short %%No_H_Flip_P0

	ALIGN32
	
	%%No_H_Flip_P0
	%%No_H_Flip_P0_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
;PRINT_TAG 61
			PUTLINE_SPRITE 0, %2					; on affiche la ligne du pattern sprite
;PRINT_TAG 62

			add ebp, byte 8							; on affiche le pattern precedent
			add esi, edi							; on va sur le prochain pattern
			cmp ebp, [Data_Spr.H_Max]				; on teste si on a fait tout les patterns du sprite
			jl near %%No_H_Flip_P0_Loop				; sinon on continue
		jmp %%End_Sprite_Loop

	ALIGN32
	
	%%No_H_Flip_P1
	%%No_H_Flip_P1_Loop
			mov ebx, [VRam + esi]					; ebx = Pattern Data
;PRINT_TAG 61
			PUTLINE_SPRITE 1, %2					; on affiche la ligne du pattern sprite
;PRINT_TAG 62

			add ebp, byte 8							; on affiche le pattern precedent
			add esi, edi							; on va sur le prochain pattern
			cmp ebp, [Data_Spr.H_Max]				; on teste si on a fait tout les patterns du sprite
			jl near %%No_H_Flip_P1_Loop				; sinon on continue
		jmp short %%End_Sprite_Loop
				
	ALIGN32
	
	%%End_Sprite_Loop
;PRINT_TAG 7
		mov edi, [Data_Misc.X]
		add edi, byte 4
		mov [Data_Misc.X], edi
		cmp edi, [Data_Misc.Borne]
		jb near %%Sprite_Loop

%%End
;PRINT_TAG 8

%endmacro


;****************************************

; macro RENDER_LINE
; param :
; %1 = 1 pour mode entrelac� et 0 sinon
; %2 = Shadow / Highlight (0 = Disable et 1 = Enable)

%macro RENDER_LINE 2

;PRINT_TAG -1
	test dword [VDP_Reg + 11 * 4], 4
	jz near %%Full_VScroll

%%Cell_VScroll
	RENDER_LINE_SCROLL_B     %1, 1, %2
;PRINT_TAG -2
	RENDER_LINE_SCROLL_A_WIN %1, 1, %2
	jmp %%Scroll_OK

%%Full_VScroll
	RENDER_LINE_SCROLL_B     %1, 0, %2
;PRINT_TAG -2
	RENDER_LINE_SCROLL_A_WIN %1, 0, %2

%%Scroll_OK
;PRINT_TAG -3
	RENDER_LINE_SPR          %1, %2
;PRINT_TAG -4

%endmacro


; *******************************************************

	DECL Render_Line

PRINT_RENDER_LINE_START

		pushad

		mov ebx, [VDP_Current_Line]
		xor eax, eax
		mov edi, [TAB336 + ebx * 4]
		test dword [VDP_Reg + 1 * 4], 0x40		; on teste si le VDP est activ�
		push edi								; on va se avoir besoin de cette valeur plus tard
		jnz short .VDP_Enable					; sinon, on n'affiche rien

;PRINT_TAG -12340002
			test byte [VDP_Reg + 12 * 4], 0x08
			cld
			mov ecx, 160
			jz short .No_Shadow

			mov eax, 0x40404040

	.No_Shadow
			lea edi, [MD_Screen + edi * 2 + 8 * 2]
			rep stosd
			jmp .VDP_OK

	ALIGN4

	.VDP_Enable
;PRINT_TAG -12340001
;push eax
;mov eax,[Data_Misc.Spr_End]
;PRINT_DWORD eax
;pop eax
		mov ebx, [VRam_Flag]
		mov eax, [VDP_Reg + 12 * 4]
		and ebx, byte 3
		and eax, byte 4
		mov byte [VRam_Flag], 0
		jmp [.Table_Sprite_Struct + ebx * 8 + eax]

	ALIGN4
	
	.Table_Sprite_Struct
		dd 	.Sprite_Struc_OK
		dd 	.Sprite_Struc_OK
		dd 	.MSS_Complete, .MSS_Complete_Interlace
		dd 	.MSS_Partial, .MSS_Partial_Interlace
		dd 	.MSS_Complete, .MSS_Complete_Interlace

	ALIGN4

	.MSS_Complete
			MAKE_SPRITE_STRUCT 0
			jmp .Sprite_Struc_OK

	ALIGN32

	.MSS_Complete_Interlace
			MAKE_SPRITE_STRUCT 1
			jmp .Sprite_Struc_OK

	ALIGN32

	.MSS_Partial
	.MSS_Partial_Interlace
			MAKE_SPRITE_STRUCT_PARTIAL
			jmp short .Sprite_Struc_OK

	ALIGN32
	
	.Sprite_Struc_OK
;PRINT_TAG -12340003
;push eax
;mov eax,[Data_Misc.Spr_End]
;PRINT_DWORD eax
;pop eax
		mov eax, [VDP_Reg + 12 * 4]
		and eax, byte 0xC
		jmp [.Table_Render_Line + eax]

	ALIGN4
	
	.Table_Render_Line
		dd 	.NHS_NInterlace
		dd 	.NHS_Interlace
		dd 	.HS_NInterlace
		dd 	.HS_Interlace
		
	ALIGN4

	.NHS_NInterlace
			RENDER_LINE 0, 0
			jmp .VDP_OK

	ALIGN32
	
	.NHS_Interlace
			RENDER_LINE 1, 0
			jmp .VDP_OK

	ALIGN32

	.HS_NInterlace
			RENDER_LINE 0, 1
			jmp .VDP_OK

	ALIGN32
	
	.HS_Interlace
			RENDER_LINE 1, 1
			jmp short .VDP_OK

	ALIGN32
	
	.VDP_OK
;PRINT_TAG -12340004
		test byte [CRam_Flag], 1		; teste si la palette a etait modifiee
		jz near .Palette_OK				; si oui

		test byte [VDP_Reg + 12 * 4], 8
		jnz near .Palette_HS

		UPDATE_PALETTE 0
		jmp .Palette_OK

	ALIGN4
		
	.Palette_HS
		UPDATE_PALETTE 1

	ALIGN4
	
	.Palette_OK
		mov ecx, 160
		mov eax, [H_Pix_Begin]
		mov edi, [esp]
		sub ecx, eax
		add esp, byte 4
		lea edi, [MD_Screen + edi * 2 + 8 * 2]
		shr ecx, 1
		mov esi, MD_Palette
		jmp short .Genesis_Loop

	ALIGN32
	
	.Genesis_Loop
			movzx eax, byte [edi + 0]
			movzx ebx, byte [edi + 2]
			movzx edx, byte [edi + 4]
			movzx ebp, byte [edi + 6]
			mov ax, [esi + eax * 2]
			mov bx, [esi + ebx * 2]
			mov dx, [esi + edx * 2]
			mov bp, [esi + ebp * 2]
			mov [edi + 0], ax
			mov [edi + 2], bx
			mov [edi + 4], dx
			mov [edi + 6], bp
			add edi, byte 8

;			movzx ebx, byte [edi + 0]
;			movzx ebp, byte [edi + 2]
;			mov ax, [esi + ebx * 2]
;			mov dx, [esi + ebp * 2]
;			mov [edi + 0], ax
;			mov [edi + 2], dx
;			add edi, byte 4

;			mov bl, byte [edi + 0]
;			mov dl, byte [edi + 2]
;			mov ax, [esi + ebx * 2]
;			mov bp, [esi + edx * 2]
;			mov [edi + 0], ax
;			mov [edi + 2], bp
;			add edi, byte 4

			dec ecx
			jnz short .Genesis_Loop

;PRINT_TAG 12345
		popad
		ret



; *******************************************************

	DECL Render_Line_32X

		ret

