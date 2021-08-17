#define ENABLE_SSG

#define S0             0  /* Stupid typo of the YM2612 */
#define S1             2
#define S2             1
#define S3             3

#define SIN_HBITS      12                // Sinus phase counter int part
#define SIN_LBITS      (26 - SIN_HBITS)          // Sinus phase counter float part (best setting)

#if(SIN_LBITS > 16)
#define SIN_LBITS      16                // Can't be greater than 16 bits
#endif

#define ENV_HBITS      12                /* Env phase counter int part */
#define ENV_LBITS      (28 - ENV_HBITS)          /* Env phase counter float part (best setting) */

#define LFO_HBITS      10                /* LFO phase counter int part */
#define LFO_LBITS      (28 - LFO_HBITS)          /* LFO phase counter float part (best setting) */

#define SIN_LENGHT     (1 << SIN_HBITS)
#define ENV_LENGHT     (1 << ENV_HBITS)
#define LFO_LENGHT     (1 << LFO_HBITS)

#define TL_LENGHT      (ENV_LENGHT * 3)          /* Env + TL scaling + LFO */

#define SIN_MASK       (SIN_LENGHT - 1)
#define ENV_MASK       (ENV_LENGHT - 1)
#define LFO_MASK       (LFO_LENGHT - 1)

#define ENV_ATTACK     ((ENV_LENGHT * 0) << ENV_LBITS)
#define ENV_DECAY      ((ENV_LENGHT * 1) << ENV_LBITS)
#define ENV_END        ((ENV_LENGHT * 2) << ENV_LBITS)

#define OUTPUT_BITS         16
#define MAX_OUT_BITS   (SIN_HBITS + SIN_LBITS + 2)    // Modulation = -4 <--> +4
#define MAX_OUT        ((1 << MAX_OUT_BITS) - 1)
#define OUT_BITS       (OUTPUT_BITS - 2)
#define OUT_SHIFT      (MAX_OUT_BITS - OUT_BITS)
//#define LIMIT_CH_OUT   ((int) (((1 << OUT_BITS) * 1.5) - 1))
//#define LIMIT_CH_OUT   ((((1 << OUT_BITS) * 1.5) - 1))
#define LIMIT_CH_OUT   98303

#define LFO_FMS_LBITS  9  // FIXED (LFO_FMS_BASE gives somethink as 1)

#define ATTACK	0
#define DECAY	1
#define SUSTAIN	2
#define RELEASE	3

//slot_ structure
#define SLOT_TL_OFS		(0)
#define SLOT_TLL_OFS		(4)
#define SLOT_SLL_OFS		(8)
#define SLOT_AMS_OFS		(12)
#define SLOT_KSR_OFS		(16)
#define SLOT_SEG_OFS		(20)
#define SLOT_Fcnt_OFS		(24)
#define SLOT_Finc_OFS		(28)
#define SLOT_Ecurp_OFS		(32)
#define SLOT_Ecnt_OFS		(36)
#define SLOT_Einc_OFS		(40)
#define SLOT_Ecmp_OFS		(44)
#define SLOT_EincA_OFS		(48)
#define SLOT_EincD_OFS		(52)
#define SLOT_EincS_OFS		(56)
#define SLOT_DT_OFS		(60)
#define SLOT_MUL_OFS		(64)
#define SLOT_PTR_AR_OFS		(68)
#define SLOT_PTR_DR_OFS		(72)
#define SLOT_PTR_SR_OFS		(76)
#define SLOT_PTR_RR_OFS		(80)
#define SLOT_EincR_OFS		(84)
#define SLOT_PTR_OUTp_OFS	(88)
#define SLOT_INd_OFS		(92)
#define SLOT_ChgEnM_OFS		(96)
#define SLOT_KSR_S_OFS		(100)
#define SLOT_AMSon_OFS		(104)
#define SLOT_SIZEOF		(108)

//channel_ structure
#define CHANNEL_S0_OUT_OFS	(0) /*[4];      // anciennes sorties slot 0 (pour le feed back) */
#define CHANNEL_S0_OUT_S0_OFS	(0+4*S0)
#define CHANNEL_S0_OUT_S1_OFS	(0+4*S1)
#define CHANNEL_S0_OUT_S2_OFS	(0+4*S2)
#define CHANNEL_S0_OUT_S3_OFS	(0+4*S3)
#define CHANNEL_Old_OUTd_OFS	(16) /* ancienne sortie de la voie (son brut)*/
#define CHANNEL_OUTd_OFS	(20) /* sortie de la voie (son brut)*/
#define CHANNEL_LEFT_OFS	(24) /* LEFT enable flag*/
#define CHANNEL_RIGHT_OFS	(28) /* RIGHT enable flag*/
#define CHANNEL_ALGO_OFS	(32) /* Algorythm = détermine les connections entre les opérateurs*/
#define CHANNEL_FB_OFS		(36) /* shift count of self feed back = degré de "Feed-Back" du SLOT 1 (il est son unique entrée)*/
#define CHANNEL_FMS_OFS		(40) /* Fréquency Modulation Sensitivity of channel = degré de modulation de la fréquence sur la voie par le LFO*/
#define CHANNEL_AMS_OFS		(44) /* Amplitude Modulation Sensitivity of channel = degré de modulation de l'amplitude sur la voie par le LFO*/
#define CHANNEL_FNUM_OFS	(48)
#define CHANNEL_FNUM_S0_OFS	(48+4*S0)
#define CHANNEL_FNUM_S1_OFS	(48+4*S1)
#define CHANNEL_FNUM_S2_OFS	(48+4*S2)
#define CHANNEL_FNUM_S3_OFS	(48+4*S3)
#define CHANNEL_FOCT_OFS	(64) /*[4]  octave de la voie (+ 3 pour le mode spécial)*/
#define CHANNEL_FOCT_S0_OFS	(64+4*S0)
#define CHANNEL_FOCT_S1_OFS	(64+4*S1)
#define CHANNEL_FOCT_S2_OFS	(64+4*S2)
#define CHANNEL_FOCT_S3_OFS	(64+4*S3)
#define CHANNEL_KC_OFS		(80) /*[4]  Key Code = valeur fonction de la fréquence (voir KSR pour les slots, KSR = KC >> KSR_S)*/
#define CHANNEL_KC_S0_OFS	(80+4*S0)
#define CHANNEL_KC_S1_OFS	(80+4*S1)
#define CHANNEL_KC_S2_OFS	(80+4*S2)
#define CHANNEL_KC_S3_OFS	(80+4*S3)
#define CHANNEL_SLOT_OFS	(96) /*[4]  four slot.operators = les 4 slots de la voie*/
#define CHANNEL_SLOT_S0_OFS	(96+SLOT_SIZEOF*S0)
#define CHANNEL_SLOT_S1_OFS	(96+SLOT_SIZEOF*S1)
#define CHANNEL_SLOT_S2_OFS	(96+SLOT_SIZEOF*S2)
#define CHANNEL_SLOT_S3_OFS	(96+SLOT_SIZEOF*S3)
#define CHANNEL_FFlag		(528) /*[4]  Frequency step recalculation flag*/
#define CHANNEL_SIZEOF		(532) /*[4]  Frequency step recalculation flag*/

.macro LD_OFS ofs, addr, dst
	//DO NOT PUT THIS MACRO IN A DELAY SLOT
	.if \ofs > 124
		.error "Offset too large"
	.endif
	.if \ofs < -128
		.error "Offset too small"
	.endif
	.if (\ofs < 64) && (\ofs >= 0)
		mov.l	@(\ofs, \addr), \dst
	.else
		.if \addr == r0
			.error "Offset so large/small address must be overwritten"
		.endif
		mov	#\ofs, r0
		mov.l	@(r0, \addr), \dst
	.endif
.endm

.macro ST_OFS val, ofs, addr
	//DO NOT PUT THIS MACRO IN A DELAY SLOT
	.if \ofs > 124
		.error "Offset too large"
	.endif
	.if \ofs < -128
		.error "Offset too small"
	.endif
	.if (\ofs < 64) && (\ofs >= 0)
		mov.l	\val, @(\ofs, \addr)
	.else
		.if \addr == r0
			.error "Offset so large/small address must be overwritten"
		.endif
		.if \val == r0
			.error "Offset so large/small value to store must be overwritten"
		.endif
		mov	#\ofs, r0
		mov.l	\val, @(r0, \addr)
	.endif
.endm

.macro LD_SP ofs, dst
	LD_OFS \ofs, r15, \dst
.endm

.macro ST_SP val, ofs
	ST_OFS \val, \ofs, r15
.endm

//Register allocated variables
#define tmp0		r1
#define tmp1		r2
#define tmp2		r3
#define channel		r4
#define dstptrptr	r5
#define length		r6
#define slot0		r7
#define slot1		r8
#define slot2		r9
#define slot3		r10
#define tmp3		r11
#define sintab		r12
#define sinmask		r13
#define sinlbits	r14

//Stack allocated variables
#define SP_BASE		(0)
#define IN0_OFS		(SP_BASE + 0)
#define IN1_OFS		(SP_BASE + 4)
#define IN2_OFS		(SP_BASE + 8)
#define IN3_OFS		(SP_BASE + 12)
#define EN0_OFS		(SP_BASE + 16)
#define EN1_OFS		(SP_BASE + 20)
#define EN2_OFS		(SP_BASE + 24)
#define EN3_OFS		(SP_BASE + 28)
#define LEFT_OFS	(SP_BASE + 32)
#define RIGHT_OFS	(SP_BASE + 36)
#define	LFO_FREQ_UP_OFS	(SP_BASE + 40)
#define	LFO_ENV_UP_OFS	(SP_BASE + 44)
#define SP_ALLOC_SIZE	(48)

//Constant values
#define ADDR_ENV_TAB		108
#define CONST_ENV_TAB		108f
#define ADDR_ENV_MASK		109
#define CONST_ENV_MASK		109f
#define ADDR_ENV_END		110
#define CONST_ENV_END		110f
#define ADDR_ENV_DECAY		111
#define CONST_ENV_DECAY		111f
#define ADDR_SIN_TAB		112
#define CONST_SIN_TAB		112f
#define ADDR_SIN_MASK		113
#define CONST_SIN_MASK		113f
#define ADDR_LFO_FREQ_UP	114
#define CONST_LFO_FREQ_UP	114f
#define ADDR_LIMIT_VAL		300
#define CONST_LIMIT_VAL		300f

.macro CONST_ENV_TABLE
.align 2
ADDR_ENV_TAB:
	.long	_ENV_TAB
ADDR_ENV_MASK:
	.long	ENV_MASK
.endm

.macro CONST_ENV_EVENT
.align 2
ADDR_ENV_END:
	.long	ENV_END
ADDR_ENV_DECAY:
	.long	ENV_DECAY
.endm

.macro CONST_SIN_TABLE
.align 2
ADDR_SIN_TAB:
	.long	_SIN_TAB
ADDR_SIN_MASK:
	.long	SIN_MASK
.endm

.macro CONST_LFO_FREQ_LOOK_UP
.align 2
ADDR_LFO_FREQ_UP:
	.long	LFO_FREQ_UP
.endm

.macro CONST_LIMIT
.align 2
ADDR_LIMIT_VAL:
	.long	LIMIT_CH_OUT
.endm


.macro GET_CURRENT_PHASE
	//8 cycles
	/*
		in0 = CH->SLOT[S0].Fcnt;
		in1 = CH->SLOT[S1].Fcnt;
		in2 = CH->SLOT[S2].Fcnt;
		in3 = CH->SLOT[S3].Fcnt;
	*/
	mov.l	@(SLOT_Fcnt_OFS, slot0), tmp0
	mov.l	@(SLOT_Fcnt_OFS, slot1), tmp1
	mov.l	tmp0, @(IN0_OFS, r15)
	mov.l	tmp1, @(IN1_OFS, r15)
	mov.l	@(SLOT_Fcnt_OFS, slot2), tmp0
	mov.l	@(SLOT_Fcnt_OFS slot3), tmp1
	mov.l	tmp0, @(IN2_OFS, r15)
	mov.l	tmp1, @(IN3_OFS, r15)
.endm

.macro UPDATE_PHASE
	//14 cycles
	/*
		CH->SLOT[S0].Fcnt += CH->SLOT[S0].Finc;
		CH->SLOT[S1].Fcnt += CH->SLOT[S1].Finc;
		CH->SLOT[S2].Fcnt += CH->SLOT[S2].Finc;
		CH->SLOT[S3].Fcnt += CH->SLOT[S3].Finc;
	*/
	mov.l	@(SLOT_Fcnt_OFS, slot0), tmp0
	mov.l	@(SLOT_Fcnt_OFS, slot1), tmp1
	mov.l	@(SLOT_Finc_OFS, slot0), tmp2
	mov.l	@(SLOT_Finc_OFS, slot1), tmp3
	add	tmp0, tmp2
	add	tmp1, tmp3
	mov.l	tmp0, @(SLOT_Fcnt_OFS, slot0)
	mov.l	tmp1, @(SLOT_Fcnt_OFS, slot1)
	
	mov.l	@(SLOT_Fcnt_OFS, slot2), tmp0
	mov.l	@(SLOT_Fcnt_OFS, slot3), tmp1
	mov.l	@(SLOT_Finc_OFS, slot2), tmp2
	mov.l	@(SLOT_Finc_OFS, slot3), tmp3
	add	tmp0, tmp2
	add	tmp1, tmp3
	mov.l	tmp0, @(SLOT_Fcnt_OFS, slot2)
	mov.l	tmp1, @(SLOT_Fcnt_OFS, slot3)
.endm

.macro GET_CURRENT_PHASE_AND_UPDATE_PHASE
	//16 cycles
	/*
		in0 = CH->SLOT[S0].Fcnt;
		in1 = CH->SLOT[S1].Fcnt;
		in2 = CH->SLOT[S2].Fcnt;
		in3 = CH->SLOT[S3].Fcnt;
		CH->SLOT[S0].Fcnt += CH->SLOT[S0].Finc;
		CH->SLOT[S1].Fcnt += CH->SLOT[S1].Finc;
		CH->SLOT[S2].Fcnt += CH->SLOT[S2].Finc;
		CH->SLOT[S3].Fcnt += CH->SLOT[S3].Finc;
	*/
	mov.l	@(SLOT_Fcnt_OFS, slot0), tmp0
	mov.l	@(SLOT_Finc_OFS, slot0), tmp2
	mov.l	@(SLOT_Fcnt_OFS, slot1), tmp1
	mov.l	@(SLOT_Finc_OFS, slot1), tmp3
	mov.l	tmp0, @(IN0_OFS, r15)
	add	tmp0, tmp2
	mov.l	tmp1, @(IN1_OFS, r15)
	add	tmp1, tmp3
	mov.l	tmp2, @(SLOT_Fcnt_OFS, slot0)
	mov.l	tmp3, @(SLOT_Fcnt_OFS, slot1)
	
	mov.l	@(SLOT_Fcnt_OFS, slot2), tmp0
	mov.l	@(SLOT_Finc_OFS, slot2), tmp2
	mov.l	@(SLOT_Fcnt_OFS, slot3), tmp1
	mov.l	@(SLOT_Finc_OFS, slot3), tmp3
	mov.l	tmp0, @(IN2_OFS, r15)
	add	tmp0, tmp2
	mov.l	tmp1, @(IN3_OFS, r15)
	add	tmp1, tmp3
	mov.l	tmp2, @(SLOT_Fcnt_OFS, slot2)
	mov.l	tmp3, @(SLOT_Fcnt_OFS, slot3)
.endm

.macro GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO_SLOT slot, in_ofs
	/*
		in0 = CH->SLOT[S0].Fcnt;
		CH->SLOT[S0].Fcnt += CH->SLOT[S0].Finc + ((CH->SLOT[S0].Finc * freq_LFO) >> LFO_FMS_LBITS);
	*/
	
	LD_OFS	SLOT_Finc_OFS, \slot, tmp0
	LD_OFS	SLOT_Fcnt_OFS, \slot, tmp2
	mul.l	tmp0, tmp1
	ST_SP	tmp2, \in_ofs
	add	tmp0, tmp2
	sts	macl, r0
	shad	sinlbits, r0
	add	r0, tmp2
	ST_OFS	tmp2, SLOT_Fcnt_OFS, \slot
.endm

.macro GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
	/*
		freq_LFO = (CH->FMS * LFO_FREQ_UP[i]) >> (LFO_HBITS - 1);
		if (freq_LFO) {
			CH->SLOT[S0].Fcnt += CH->SLOT[S0].Finc + ((CH->SLOT[S0].Finc * freq_LFO) >> LFO_FMS_LBITS);
			CH->SLOT[S1].Fcnt += CH->SLOT[S1].Finc + ((CH->SLOT[S1].Finc * freq_LFO) >> LFO_FMS_LBITS);
			CH->SLOT[S2].Fcnt += CH->SLOT[S2].Finc + ((CH->SLOT[S2].Finc * freq_LFO) >> LFO_FMS_LBITS);
			CH->SLOT[S3].Fcnt += CH->SLOT[S3].Finc + ((CH->SLOT[S3].Finc * freq_LFO) >> LFO_FMS_LBITS);
		} else {
			CH->SLOT[S0].Fcnt += CH->SLOT[S0].Finc;
			CH->SLOT[S1].Fcnt += CH->SLOT[S1].Finc;
			CH->SLOT[S2].Fcnt += CH->SLOT[S2].Finc;
			CH->SLOT[S3].Fcnt += CH->SLOT[S3].Finc;
		}
	*/

	mov	#-(LFO_HBITS-1), tmp2
	LD_SP	LFO_FREQ_UP_OFS, tmp0
	
	LD_OFS	CHANNEL_FMS_OFS, channel, tmp1
	
	mov.l	@tmp0+, tmp3
	
	ST_SP	tmp0, LFO_FREQ_UP_OFS
	
	mul.l	tmp1, tmp3
	
	sts	macl, tmp1
	
	shad	tmp2, tmp1
	
	tst	tmp1, tmp1
	
	//bt	1f
	mov	#-LFO_FMS_LBITS, sinlbits
	GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO_SLOT	slot0, IN0_OFS
	GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO_SLOT	slot1, IN1_OFS
	GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO_SLOT	slot2, IN2_OFS
	GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO_SLOT	slot3, IN3_OFS

	bra	2f
	 nop
1:
	GET_CURRENT_PHASE_AND_UPDATE_PHASE
2:
.endm



.macro GET_CURRENT_ENV_SLOT slot, en_ofs
	//8 cycles
	/*
		slot = slot0, slot1, slot2, slot3
		en_ofs = EN0_OFS, EN1_OFS, EN2_OFS, EN3_OFS
		env_tab  = _ENV_TAB
		env_mask = ENV_MASK
	
		en0 = ENV_TAB[CH->SLOT[S0].Ecnt >> ENV_LBITS] + CH->SLOT[S0].TLL;
		//if(CH->SLOT[S0].SEG & 4)
		//	en0 = (en0 > ENV_MASK) ? (0) : (en0 ^ ENV_MASK);
	*/
	LD_OFS	SLOT_Ecnt_OFS, \slot, tmp0
	
	mov.l	@(SLOT_TLL_OFS, \slot), tmp2
	
	shad	sinlbits, tmp0
	
	shll2	tmp0
	mov	sintab, r0
	
	mov.l	@(r0, tmp0), tmp0
	
#ifdef ENABLE_SSG	
	mov.l	@(SLOT_SEG_OFS, \slot), r0
#endif
	
	add	tmp2, tmp0

#ifdef ENABLE_SSG
	tst	#4, r0
	
	bt	1f		//TODO check me
	
	cmp/gt	tmp0, sinmask
	xor	sinmask, tmp0
	
	bt	1f		//TODO check me
	mov	#0, tmp0
	
1:
#endif
	mov.l	tmp0, @(\en_ofs, r15)
.endm



.macro GET_CURRENT_ENV_SLOT_PAIR slota, ena_ofs, slotb, enb_ofs
	//10 cycles without SSG-EG?
	/*
		en0 = ENV_TAB[CH->SLOT[S0].Ecnt >> ENV_LBITS] + CH->SLOT[S0].TLL;
		//if(CH->SLOT[S0].SEG & 4)
		//	en0 = (en0 > ENV_MASK) ? (0) : (en0 ^ ENV_MASK);
	*/
	LD_OFS	SLOT_Ecnt_OFS, \slota, tmp0
	
	mov.l	@(SLOT_TLL_OFS, \slota), tmp2
	
	LD_OFS	SLOT_Ecnt_OFS, \slotb, tmp1
	shad	sinlbits, tmp0
	
	shll2	tmp0
	mov.l	@(SLOT_TLL_OFS, \slotb), tmp3
	
	shad	sinlbits, tmp1
	mov	tmp0, r0
	
	mov.l	@(r0, sintab), tmp0
	shll2	tmp1
	
	mov	tmp1, r0
	mov.l	@(r0, sintab), tmp1
	
#ifdef ENABLE_SSG	
	mov.l	@(SLOT_SEG_OFS, \slota), r0
#endif
	add	tmp2, tmp0
	
	add	tmp3, tmp1
	mov.l	tmp0, @(\ena_ofs, r15)

#ifdef ENABLE_SSG
	tst	#4, r0
	mov.l	@(SLOT_SEG_OFS, \slotb), r0
	
	bt/s	2f		//TODO check me
	 tst	#4, r0
	
	cmp/gt	tmp0, sinmask
	xor	sinmask, tmp0
	
	bt	1f		//TODO check me
	mov	#0, tmp0
	
1:
	tst	#4, r0
2:
	bt	1f		//TODO check me
	
	cmp/gt	tmp1, sinmask
	xor	sinmask, tmp1
	
	bt	1f		//TODO check me
	mov	#0, tmp1
	
1:
#endif
	mov.l	tmp1, @(\enb_ofs, r15)
.endm



.macro GET_CURRENT_ENV_LFO_SLOT slot, en_ofs
	//11 cycles?
	/*
		if(CH->SLOT[S0].SEG & 4) {
			en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL
			if(en0 > ENV_MASK)
				en0 = 0;
			else
				en0 = (en0 ^ ENV_MASK) + (env_LFO >> CH->SLOT[S0].AMS);
		} else {
			en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL + (env_LFO >> CH->SLOT[S0].AMS);
		}
		-----------------------
		en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL
		slotlfo = env_LFO >> CH->SLOT[S0].AMS;
		if(CH->SLOT[S0].SEG & 4) {
			en0 = (en0 > ENV_MASK) ? 0 : (en0 ^ ENV_MASK) + slotlfo;
		} else {
			en0 += slotlfo;
		}
	*/
	//env_LFO is in dstptrptr
	//ENV_LBITS is in sinlbits
	//ENV_MASK is in sinmask
	//ENV_TAB is in sintab
	LD_OFS	SLOT_Ecnt_OFS, \slot, r0
	LD_OFS	SLOT_TLL_OFS, \slot, tmp0
	LD_OFS	SLOT_AMS_OFS, \slot, tmp2
	
	shad	sinlbits, r0
	shll2	r0
	
	mov.l	@(r0, sintab), tmp1
	
	neg	tmp2, tmp2
#ifdef ENABLE_SSG
	LD_OFS	SLOT_SEG_OFS, \slot, r0
#endif
	mov	dstptrptr, tmp3

	shad	tmp2, tmp3
#ifdef ENABLE_SSG	
	tst	#4, r0
	
	bt/s	1f
	 add	tmp0, tmp1
	
	cmp/gt	tmp1, sinmask
	xor	sinmask, tmp1
	
	bt	1f		//TODO check me
	bra	2f
	 mov	#0, tmp1
	
1:
	add	tmp3, tmp1
2:
#else
	add	tmp0, tmp1
	add	tmp3, tmp1
#endif
	
	ST_SP	tmp1, \en_ofs
.endm

.macro GET_CURRENT_ENV_LFO
	/*
	env_LFO = LFO_ENV_UP[i];
	if(CH->SLOT[S0].SEG & 4) {
		en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL
		if(en0 > ENV_MASK)
			en0 = 0;
		else
			en0 = (en0 ^ ENV_MASK) + (env_LFO >> CH->SLOT[S0].AMS);
	} else {
		en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL + (env_LFO >> CH->SLOT[S0].AMS);
	}
	if(CH->SLOT[S1].SEG & 4) {
		en1 = ENV_TAB[(CH->SLOT[S1].Ecnt >> ENV_LBITS)] + CH->SLOT[S1].TLL
		if(en1 > ENV_MASK)
			en1 = 0;
		else
			en1 = (en1 ^ ENV_MASK) + (env_LFO >> CH->SLOT[S1].AMS);
	} else {
		en1 = ENV_TAB[(CH->SLOT[S1].Ecnt >> ENV_LBITS)] + CH->SLOT[S1].TLL + (env_LFO >> CH->SLOT[S1].AMS);
	}
	if(CH->SLOT[S2].SEG & 4) {
		en2 = ENV_TAB[(CH->SLOT[S2].Ecnt >> ENV_LBITS)] + CH->SLOT[S2].TLL
		if(en2 > ENV_MASK)
			en2 = 0;
		else
			en2 = (en2 ^ ENV_MASK) + (env_LFO >> CH->SLOT[S2].AMS);
	} else {
		en2 = ENV_TAB[(CH->SLOT[S2].Ecnt >> ENV_LBITS)] + CH->SLOT[S2].TLL + (env_LFO >> CH->SLOT[S2].AMS);
	}
	if(CH->SLOT[S3].SEG & 4) {
		en3 = ENV_TAB[(CH->SLOT[S3].Ecnt >> ENV_LBITS)] + CH->SLOT[S3].TLL
		if(en3 > ENV_MASK)
			en3 = 0;
		else
			en3 = (en3 ^ ENV_MASK) + (env_LFO >> CH->SLOT[S3].AMS);
	} else {
		en3 = ENV_TAB[(CH->SLOT[S3].Ecnt >> ENV_LBITS)] + CH->SLOT[S3].TLL + (env_LFO >> CH->SLOT[S3].AMS);
	}
	*/
	
	//env_LFO = LFO_ENV_UP[i];
	LD_SP	LFO_ENV_UP_OFS, tmp0
	mov.l	CONST_ENV_TAB, sintab
	mov.l	CONST_ENV_MASK, sinmask
	mov.l	@tmp0+, dstptrptr
	ST_SP	tmp0, LFO_ENV_UP_OFS
#ifdef ENABLE_SSG
	bra	1f
	 mov	#-ENV_LBITS, sinlbits
	CONST_ENV_TABLE
1:
#else
	 mov	#-ENV_LBITS, sinlbits
#endif
	
	GET_CURRENT_ENV_LFO_SLOT	slot0, EN0_OFS
	GET_CURRENT_ENV_LFO_SLOT	slot1, EN1_OFS
	GET_CURRENT_ENV_LFO_SLOT	slot2, EN2_OFS
	GET_CURRENT_ENV_LFO_SLOT	slot3, EN3_OFS
.endm

.macro GET_CURRENT_ENV
	/*
		if(CH->SLOT[S0].SEG & 4) {
			  if((en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL) > ENV_MASK) en0 = 0;
			  else en0 ^= ENV_MASK;
		} else en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL; 
		
		if(CH->SLOT[S0].SEG & 4) {
			en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL;
			if (en0 > ENV_MASK)
				en0 = 0;
			else
				en0 ^= ENV_MASK;
		} else {
			en0 = ENV_TAB[(CH->SLOT[S0].Ecnt >> ENV_LBITS)] + CH->SLOT[S0].TLL;
		}
		
		if(CH->SLOT[S1].SEG & 4) {
			en1 = ENV_TAB[(CH->SLOT[S1].Ecnt >> ENV_LBITS)] + CH->SLOT[S1].TLL;
			if (enl > ENV_MASK)
				en1 = 0;
			else
				en1 ^= ENV_MASK;
		} else {
			en1 = ENV_TAB[(CH->SLOT[S1].Ecnt >> ENV_LBITS)] + CH->SLOT[S1].TLL;
		}

		if(CH->SLOT[S2].SEG & 4) {
			en2 = ENV_TAB[(CH->SLOT[S2].Ecnt >> ENV_LBITS)] + CH->SLOT[S2].TLL;
			if(en2 > ENV_MASK)
				en2 = 0;
			else
				en2 ^= ENV_MASK;
		} else {
			en2 = ENV_TAB[(CH->SLOT[S2].Ecnt >> ENV_LBITS)] + CH->SLOT[S2].TLL;
		}

		if(CH->SLOT[S3].SEG & 4) {
			en3 = ENV_TAB[(CH->SLOT[S3].Ecnt >> ENV_LBITS)] + CH->SLOT[S3].TLL;
			if(en3 > ENV_MASK)
				en3 = 0;
			else
				en3 ^= ENV_MASK;
		} else {
			en3 = ENV_TAB[(CH->SLOT[S3].Ecnt >> ENV_LBITS)] + CH->SLOT[S3].TLL;
		}
	*/

	mov.l	CONST_ENV_TAB, sintab
	mov.l	CONST_ENV_MASK, sinmask
	mov	#-ENV_LBITS, sinlbits
#if 0
	GET_CURRENT_ENV_SLOT	slot0, EN0_OFS
	GET_CURRENT_ENV_SLOT	slot1, EN1_OFS
	GET_CURRENT_ENV_SLOT	slot2, EN2_OFS
	GET_CURRENT_ENV_SLOT	slot3, EN3_OFS
#else
	GET_CURRENT_ENV_SLOT_PAIR	slot0, EN0_OFS, slot1, EN1_OFS
	GET_CURRENT_ENV_SLOT_PAIR	slot2, EN2_OFS, slot3, EN3_OFS
#endif
.endm

.macro ENV_ATTACK_NEXT slot, jmpdst
	//8 cycles
	/*
		// Verified with Gynoug even in HQ (explode SFX)
		SL->Ecnt = ENV_DECAY;

		SL->Einc = SL->EincD;
		SL->Ecmp = SL->SLL;
		SL->Ecurp = DECAY;
	*/
	mov.l	CONST_ENV_DECAY, tmp0	//ENV_DECAY
	LD_OFS	SLOT_EincD_OFS, \slot, tmp1
	LD_OFS	SLOT_SLL_OFS, \slot, tmp2
	mov	#DECAY, tmp3
	ST_OFS	tmp0, SLOT_Ecnt_OFS, \slot
	ST_OFS	tmp1, SLOT_Einc_OFS, \slot
	ST_OFS	tmp2, SLOT_Ecmp_OFS, \slot
	bra	\jmpdst
	 mov.l	tmp3, @(SLOT_Ecurp_OFS, \slot)
.endm

.macro ENV_DECAY_NEXT slot, jmpdst
	//8 cycles
	/*
		// Verified with Gynoug even in HQ (explode SFX)
		SL->Ecnt = SL->SLL;

		SL->Einc = SL->EincS;
		SL->Ecmp = ENV_END;
		SL->Ecurp = SUBSTAIN;
	*/
	LD_OFS	SLOT_SLL_OFS, \slot, tmp0
	LD_OFS	SLOT_EincS_OFS, \slot, tmp1
	mov.l	CONST_ENV_END, tmp2	//ENV_END
	mov	#SUSTAIN, tmp3
	ST_OFS	tmp0, SLOT_Ecnt_OFS, \slot
	ST_OFS	tmp1, SLOT_Einc_OFS, \slot
	ST_OFS	tmp2, SLOT_Ecmp_OFS, \slot
	bra	\jmpdst
	 mov.l	tmp3, @(SLOT_Ecurp_OFS, \slot)
.endm

.macro ENV_SUSTAIN_NEXT slot, jmpdst
	//8 cycles no ssg
	/*
		if(YM2612_Enable_SSGEG)	{
			if(SL->SEG & 8)	{  // SSG envelope type
				if(SL->SEG & 1) {
					SL->Ecnt = ENV_END;
					SL->Einc = 0;
					SL->Ecmp = ENV_END + 1;
				} else {
					// re KEY ON

					// SL->Fcnt = 0;
					// SL->ChgEnM = 0xFFFFFFFF;

					SL->Ecnt = 0;
					SL->Einc = SL->EincA;
					SL->Ecmp = ENV_DECAY;
					SL->Ecurp = ATTACK;
				}

				SL->SEG ^= (SL->SEG & 2) << 1;
			} else {
				SL->Ecnt = ENV_END;
				SL->Einc = 0;
				SL->Ecmp = ENV_END + 1;
			}
		} else {
			SL->Ecnt = ENV_END;
			SL->Einc = 0;
			SL->Ecmp = ENV_END + 1;
		}
		
		/////////////////////////////
		SL->Ecnt = ENV_END;
		SL->Einc = 0;
		SL->Ecmp = ENV_END + 1;
		//if(YM2612_Enable_SSGEG && SL->SEG & 8) {
		//	if(!(SL->SEG & 1)) {
		//		SL->Ecnt = 0;
		//		SL->Einc = SL->EincA;
		//		SL->Ecmp = ENV_DECAY;
		//		SL->Ecurp = ATTACK;
		//	}
		//
		//	SL->SEG ^= (SL->SEG & 2) << 1;
		//}
	*/
#ifdef ENABLE_SSG
	LD_OFS	SLOT_SEG_OFS, \slot, r0
	mov.l	CONST_ENV_END, tmp0	//ENV_END
	mov	#0, tmp1
	mov	tmp0, tmp2
	add	#1, tmp2
	tst	#8, r0
	ST_OFS	tmp0, SLOT_Ecnt_OFS, \slot
	bf	1f
	ST_OFS	tmp1, SLOT_Einc_OFS, \slot
	ST_OFS	tmp2, SLOT_Ecmp_OFS, \slot
	bra	\jmpdst
	 nop
1:
	tst	#1, r0
	mov	r0, tmp3
	and	#2, r0
	add	r0, r0
	xor	r0, tmp3
	ST_OFS	tmp3, SLOT_SEG_OFS, \slot
	bt	\jmpdst

	mov	#0, tmp0
	LD_OFS	SLOT_EincA_OFS, \slot, tmp1
	mov.l	CONST_ENV_DECAY, tmp2	//ENV_DECAY
	mov	#ATTACK, tmp3
	ST_OFS	tmp0, SLOT_Ecnt_OFS, \slot
	ST_OFS	tmp1, SLOT_Einc_OFS, \slot
	ST_OFS	tmp2, SLOT_Ecmp_OFS, \slot
	ST_OFS	tmp3, SLOT_Ecurp_OFS, \slot
	bra	\jmpdst
	 nop
#else
	mov.l	CONST_ENV_END, tmp0	//ENV_END
	mov	#0, tmp1
	mov	tmp0, tmp2
	add	#1, tmp2
	ST_OFS	tmp0, SLOT_Ecnt_OFS, \slot
	ST_OFS	tmp1, SLOT_Einc_OFS, \slot
	ST_OFS	tmp2, SLOT_Ecmp_OFS, \slot
	bra	\jmpdst
	 nop
#endif
	
.endm

.macro ENV_RELEASE_NEXT slot, jmpdst
	/*
		SL->Ecnt = ENV_END;
		SL->Einc = 0;
		SL->Ecmp = ENV_END + 1;
	*/
	mov.l	CONST_ENV_END, tmp0	//ENV_END
	mov	#0, tmp1
	mov	tmp0, tmp2
	add	#1, tmp2
	ST_OFS	tmp0, SLOT_Ecnt_OFS, \slot
	ST_OFS	tmp1, SLOT_Einc_OFS, \slot
	ST_OFS	tmp2, SLOT_Ecmp_OFS, \slot
	bra	\jmpdst
	 nop
.endm

.macro ENV_NEXT_EVENT slot, Ecurp, continue
	.if \Ecurp <> r0
		mov	\Ecurp, r0
	.endif
	cmp/eq	#ATTACK, r0
	bt	100f
	cmp/eq	#DECAY, r0
	bt	101f
	cmp/eq	#SUSTAIN, r0
	bt	102f
	cmp/eq	#RELEASE, r0
	bt	103f
	bf	\continue
	//bra	\continue
	// nop
100:
	ENV_ATTACK_NEXT \slot, \continue
101:
	ENV_DECAY_NEXT \slot, \continue
102:
	ENV_SUSTAIN_NEXT \slot, \continue
103:
	ENV_RELEASE_NEXT \slot, \continue
.endm

.macro ENV_NEXT_EVENT_2 slot, continue
	LD_OFS	SLOT_Ecurp_OFS, \slot, r0
	ENV_NEXT_EVENT	\slot, r0, \continue
.endm

.macro UPDATE_ENV_SLOT slot
	//7 cycles if no event
	/*
		CH->SLOT[S0].Ecnt += CH->SLOT[S0].Einc;
		if(CH->SLOT[S0].Ecnt >= CH->SLOT[S0].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S0].Ecurp](&(CH->SLOT[S0]));
	*/
	mov.l	@(SLOT_Ecnt_OFS, \slot), tmp0
	
	LD_OFS	SLOT_Einc_OFS, \slot, tmp1
	
	LD_OFS	SLOT_Ecmp_OFS, \slot, tmp2
	
	add	tmp1, tmp0
	mov.l	@(SLOT_Ecurp_OFS, \slot), r0	//Be careful to not overwrite r0 with LD_OFS
	
	cmp/ge	tmp2, tmp0
	mov.l	tmp0, @(SLOT_Ecnt_OFS, \slot)	//Be careful to not overwrite r0 with LD_OFS
	
	bf	55f	//TODO check me
	ENV_NEXT_EVENT \slot, r0, 55f
55:
.endm



.macro UPDATE_ENV_SLOT_PAIR slota, slotb
	//12 cycles
	/*
		CH->SLOT[S0].Ecnt += CH->SLOT[S0].Einc;
		if(CH->SLOT[S0].Ecnt >= CH->SLOT[S0].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S0].Ecurp](&(CH->SLOT[S0]));
		CH->SLOT[S1].Ecnt += CH->SLOT[S1].Einc
		if(CH->SLOT[S1].Ecnt >= CH->SLOT[S1].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S1].Ecurp](&(CH->SLOT[S1]));
		CH->SLOT[S2].Ecnt += CH->SLOT[S2].Einc;
		if(CH->SLOT[S2].Ecnt >= CH->SLOT[S2].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S2].Ecurp](&(CH->SLOT[S2]));
		CH->SLOT[S3].Ecnt += CH->SLOT[S3].Einc;
		if(CH->SLOT[S3].Ecnt >= CH->SLOT[S3].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S3].Ecurp](&(CH->SLOT[S3]));
	*/
	
	mov.l	@(SLOT_Ecnt_OFS, \slota), tmp0
	
	LD_OFS	SLOT_Einc_OFS, \slota, tmp1
	
	LD_OFS	SLOT_Ecmp_OFS, \slota, tmp2
	
	add	tmp1, tmp0
	LD_OFS	SLOT_Einc_OFS, \slotb, dstptrptr
	
	cmp/ge	tmp2, tmp0
	mov.l	tmp0, @(SLOT_Ecnt_OFS, \slota)
	
	bt	70f	//TODO check me
60:
	mov.l	@(SLOT_Ecnt_OFS, \slotb), tmp0
	
	LD_OFS	SLOT_Ecmp_OFS, \slotb, tmp2
	
	add	dstptrptr, tmp0
	
	cmp/ge	tmp2, tmp0
	mov.l	tmp0, @(SLOT_Ecnt_OFS, \slotb)
	
	bt	71f
	
	bra	72f
	 nop
	
70:
	ENV_NEXT_EVENT_2	\slota, 60b
71:
	ENV_NEXT_EVENT_2	\slotb, 72f
72:
	
.endm

#if 0
.globl _YMTest
_YMTest:
UPDATE_ENV_SLOT_PAIR slot0, slot1
CONST_ENV_EVENT
#endif

.macro UPDATE_ENV
	/*
		CH->SLOT[S0].Ecnt += CH->SLOT[S0].Einc;
		if(CH->SLOT[S0].Ecnt >= CH->SLOT[S0].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S0].Ecurp](&(CH->SLOT[S0]));
		CH->SLOT[S1].Ecnt += CH->SLOT[S1].Einc
		if(CH->SLOT[S1].Ecnt >= CH->SLOT[S1].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S1].Ecurp](&(CH->SLOT[S1]));
		CH->SLOT[S2].Ecnt += CH->SLOT[S2].Einc;
		if(CH->SLOT[S2].Ecnt >= CH->SLOT[S2].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S2].Ecurp](&(CH->SLOT[S2]));
		CH->SLOT[S3].Ecnt += CH->SLOT[S3].Einc;
		if(CH->SLOT[S3].Ecnt >= CH->SLOT[S3].Ecmp)
			ENV_NEXT_EVENT[CH->SLOT[S3].Ecurp](&(CH->SLOT[S3]));
	*/
#if 1
	UPDATE_ENV_SLOT	slot0
	UPDATE_ENV_SLOT	slot1
	UPDATE_ENV_SLOT	slot2
	UPDATE_ENV_SLOT	slot3
#else
	UPDATE_ENV_SLOT_PAIR	slot0, slot1
	UPDATE_ENV_SLOT_PAIR	slot2, slot3
#endif
.endm

.macro LOAD_SINTAB_AND_SINMASK
	mov.l	CONST_SIN_TAB, sintab
	mov.l	CONST_SIN_MASK, sinmask
.endm

.macro SIN_TAB phase, env, dst
	//Trashes phase, env, r0
	//SIN_TAB[(phase >> SIN_LBITS) & SIN_MASK][env];
	shad	sinlbits, \phase
	
	and	sinmask, \phase
	
	shll2	\phase
	
	.if \phase <> r0
		mov	\phase, r0
	.endif
	mov.l	@(r0, sintab), r0
	
	shll2	\env
	
	mov.l	@(r0, \env), \dst
.endm


.macro DO_FEEDBACK
	/*
		in0 += (CH->S0_OUT[0] + CH->S0_OUT[1]) >> CH->FB;
		CH->S0_OUT[1] = CH->S0_OUT[0];
		CH->S0_OUT[0] = SIN_TAB[(in0 >> SIN_LBITS) & SIN_MASK][en0];
	*/
	
	mov	#-SIN_LBITS, sinlbits
	
	//in0 += (CH->S0_OUT[0] + CH->S0_OUT[1]) >> CH->FB;
	LD_OFS	CHANNEL_FB_OFS, channel, tmp3
	LD_OFS	CHANNEL_S0_OUT_OFS, channel, tmp1
	neg	tmp3, tmp3
	LD_OFS	CHANNEL_S0_OUT_OFS+4, channel, tmp2
	LD_SP	IN0_OFS, tmp0
	add	tmp1, tmp2	//CH->S0_OUT[0] + CH->S0_OUT[1]
	shad	tmp3, tmp2	//(CH->S0_OUT[0] + CH->S0_OUT[1]) >> CH->FB
	add	tmp2, tmp0	//in0 += (CH->S0_OUT[0] + CH->S0_OUT[1]) >> CH->FB;
	
	//CH->S0_OUT[1] = CH->S0_OUT[0];
	ST_OFS	tmp1, CHANNEL_S0_OUT_OFS+4, channel
	
	//CH->S0_OUT[0] = SIN_TAB[(in0 >> SIN_LBITS) & SIN_MASK][en0];
	shad	sinlbits, tmp0	//(in0 >> SIN_LBITS)
	and	sinmask, tmp0	//(in0 >> SIN_LBITS) & SIN_MASK
	
	shll2	tmp0
	mov	tmp0, r0
	mov.l	@(r0, sintab), r0	//SIN_TAB[(in0 >> SIN_LBITS) & SIN_MASK]
	
	LD_SP	EN0_OFS, tmp0
	shll2	tmp0
	mov.l	@(r0, tmp0), r0		//SIN_TAB[(in0 >> SIN_LBITS) & SIN_MASK][en0]

	ST_OFS	r0, CHANNEL_S0_OUT_OFS, channel	//CH->S0_OUT[0] = SIN_TAB[(in0 >> SIN_LBITS) & SIN_MASK][en0];
.endm

.macro DO_ALGO_0
	/*
		DO_FEEDBACK
		in1 += CH->S0_OUT[1];
		in2 += SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1];
		in3 += SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
		CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	*/
	
	DO_FEEDBACK

	//CH->S0_OUT[1] is in tmp1
	//CH->S0_OUT[0] is in r0
	//in1 += CH->S0_OUT[1];
	LD_SP	IN1_OFS, tmp0
	add	tmp0, tmp1
	
	//in1 is in tmp1
	//in2 += SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1];
	LD_SP	EN1_OFS, tmp2
	LD_SP	IN2_OFS, tmp3
	SIN_TAB	tmp1, tmp2, tmp1
	add	tmp3, tmp1
	
	//in2 is in tmp1
	//in3 += SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
	LD_SP	EN2_OFS, tmp2
	LD_SP	IN3_OFS, tmp3
	SIN_TAB	tmp1, tmp2, tmp1
	add	tmp3, tmp1
	
	//in3 is in tmp1
	//CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	LD_SP	EN3_OFS, tmp2
	mov	#-OUT_SHIFT, dstptrptr
	SIN_TAB	tmp1, tmp2, tmp1
	
	shad	dstptrptr, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
.endm

.macro DO_ALGO_1
	/*
		DO_FEEDBACK
		in2 += CH->S0_OUT[1] + SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1];
		in3 += SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
		CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	*/
	
	DO_FEEDBACK
	
	//CH->S0_OUT[1] is in tmp1
	//in2 += CH->S0_OUT[1] + SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1];
	LD_SP	IN1_OFS, tmp0
	LD_SP	EN1_OFS, tmp2
	LD_SP	IN2_OFS, tmp3
	SIN_TAB	tmp0, tmp2, tmp0
	add	tmp0, tmp1
	add	tmp3, tmp1
	
	//in3 += SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
	LD_SP	IN3_OFS, tmp0
	LD_SP	EN2_OFS, tmp2
	SIN_TAB	tmp1, tmp2, tmp1
	add	tmp0, tmp1
	
	//CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	LD_SP	EN3_OFS, tmp2
	SIN_TAB	tmp1, tmp2, tmp1
	mov	#-OUT_SHIFT, tmp0
	shad	tmp0, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
.endm

.macro DO_ALGO_2
	/*
		DO_FEEDBACK
		in2 += SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1];
		in3 += CH->S0_OUT[1] + SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
		CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	*/
	
	DO_FEEDBACK
	
	//CH->S0_OUT[1] is in tmp1
	mov	tmp1, dstptrptr

	LD_SP	IN1_OFS, tmp1
	//in2 += SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1];
	LD_SP	EN1_OFS, tmp2
	shad	sinlbits, tmp1
	LD_SP	IN2_OFS, tmp3
	and	sinmask, tmp1
	shll2	tmp1
	mov	tmp1, r0
	mov.l	@(r0, sintab), r0
	shll2	tmp2
	mov.l	@(r0, tmp2), tmp1
	add	tmp3, tmp1
	
	//in3 += CH->S0_OUT[1] + SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
	LD_SP	EN2_OFS, tmp2
	shad	sinlbits, tmp1
	LD_SP	IN3_OFS, tmp3
	and	sinmask, tmp1
	shll2	tmp1
	mov	tmp1, r0
	mov.l	@(r0, sintab), r0
	shll2	tmp2
	mov.l	@(r0, tmp2), tmp1
	add	tmp3, tmp1
	add	dstptrptr, tmp1
	
	//CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	LD_SP	EN3_OFS, tmp2
	shad	sinlbits, tmp1
	and	sinmask, tmp1
	shll2	tmp1
	mov	tmp1, r0
	mov.l	@(r0, sintab), r0
	shll2	tmp2
	mov	#-OUT_SHIFT, tmp0
	mov.l	@(r0, tmp2), tmp1
	
	shad	tmp0, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
.endm


.macro DO_LIMIT
	/*
		if (CH->OUTd > LIMIT_CH_OUT)
			CH->OUTd = LIMIT_CH_OUT;
		//else if (CH->OUTd < -LIMIT_CH_OUT)
		else if (-LIMIT_CH_OUT > CH->OUTd)
			CH->OUTd = -LIMIT_CH_OUT;
	*/
#if 1
	mov.l	CONST_LIMIT_VAL, r0
	
	//
	
	neg	r0, dstptrptr
	cmp/gt	r0, tmp1
	
	bf/s	1f
	 cmp/gt	tmp1, dstptrptr
	
	bra	2f
	 mov	r0, tmp1
.align 2
1:
	bf	2f
	mov	dstptrptr, tmp1
2:
//	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
#endif
.endm

.macro DO_ALGO_3
	/*
		DO_FEEDBACK
		in1 += CH->S0_OUT[1];
		in3 += SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1] + SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
		CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	*/
	DO_FEEDBACK
	
	//CH->S0_OUT[1] is in tmp1
	//in1 += CH->S0_OUT[1];
	LD_SP	IN1_OFS, tmp0
	add	tmp0, tmp1
	
	//in3 += SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1] + SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
	LD_SP	EN1_OFS, tmp2
	LD_SP	IN3_OFS, tmp3
	SIN_TAB	tmp1, tmp2, tmp1
	add	tmp3, tmp1
	
	LD_SP	IN2_OFS, tmp3
	LD_SP	EN2_OFS, tmp2
	SIN_TAB	tmp3, tmp2, tmp3
	add	tmp3, tmp1
	
	//CH->OUTd = (SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]) >> OUT_SHIFT;
	LD_SP	EN3_OFS, tmp2
	SIN_TAB	tmp1, tmp2, tmp1
	mov	#-OUT_SHIFT, dstptrptr
	shad	dstptrptr, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
.endm

.macro DO_ALGO_4
	/*
		DO_FEEDBACK
		in1 += CH->S0_OUT[1];
		in3 += SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
		CH->OUTd = ((int) SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3] + (int) SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1]) >> OUT_SHIFT;
		DO_LIMIT
	*/
	DO_FEEDBACK
	
	//CH->S0_OUT[1] is in tmp1
	//in1 += CH->S0_OUT[1];
	LD_SP	IN1_OFS, tmp0
	add	tmp0, tmp1
	
	//in3 += SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2];
	LD_SP	IN2_OFS, tmp0
	LD_SP	EN2_OFS, tmp2
	LD_SP	IN3_OFS, tmp3
	SIN_TAB	tmp0, tmp2, tmp0
	add	tmp3, tmp0
	
	//CH->OUTd = ((int) SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3] + (int) SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1]) >> OUT_SHIFT;
	LD_SP	EN3_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	LD_SP	EN1_OFS, tmp2
	SIN_TAB	tmp1, tmp2, tmp1
	mov	#-OUT_SHIFT, dstptrptr
	add	tmp0, tmp1
	shad	dstptrptr, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
	
	DO_LIMIT
.endm

.macro DO_ALGO_5
	/*
		DO_FEEDBACK
		in1 += CH->S0_OUT[1];
		in2 += CH->S0_OUT[1];
		in3 += CH->S0_OUT[1];
		CH->OUTd = ((int) SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3] + (int) SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1] + (int) SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2]) >> OUT_SHIFT;
		DO_LIMIT
	*/
	DO_FEEDBACK
	
	//CH->S0_OUT[1] is in tmp1
	//SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]
	LD_SP	IN3_OFS, tmp0
	add	tmp1, tmp0
	LD_SP	EN3_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	
	//+  SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1]
	LD_SP	IN1_OFS, tmp3
	add	tmp1, tmp3
	LD_SP	EN1_OFS, tmp2
	SIN_TAB	tmp3, tmp2, tmp3
	add	tmp3, tmp0
	
	//+  SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2]
	LD_SP	IN2_OFS, tmp3
	add	tmp1, tmp3
	LD_SP	EN2_OFS, tmp2
#if 0
	SIN_TAB	tmp3, tmp2, tmp1
	mov	#-OUT_SHIFT, dstptrptr
	add	tmp3, tmp1
#else
	SIN_TAB	tmp3, tmp2, tmp3
	mov	#-OUT_SHIFT, dstptrptr
	mov	tmp0, tmp1
	add	tmp3, tmp1
#endif
	shad	dstptrptr, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
	
	DO_LIMIT
.endm

.macro DO_ALGO_6
	/*
		DO_FEEDBACK
		in1 += CH->S0_OUT[1];
		CH->OUTd = ((int) SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3] + (int) SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1] + (int) SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2]) >> OUT_SHIFT;
		DO_LIMIT
	*/
	
	DO_FEEDBACK
	
	//CH->S0_OUT[1] is in tmp1
	//in1 += CH->S0_OUT[1];
	LD_SP	IN1_OFS, tmp0
	add	tmp1, tmp0
	
	//SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1]
	LD_SP	EN1_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp1
	
	//+ SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2]
	LD_SP	IN2_OFS, tmp0
	LD_SP	EN2_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	add	tmp0, tmp1
	
	//+ SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]
	LD_SP	IN3_OFS, tmp0
	LD_SP	EN3_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	mov	#-OUT_SHIFT, dstptrptr
	add	tmp0, tmp1
	
	shad	dstptrptr, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
	
	DO_LIMIT
.endm

.macro DO_ALGO_7
	/*
		DO_FEEDBACK
		CH->OUTd = ((int) SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3] + (int) SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1] + (int) SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2] + CH->S0_OUT[1]) >> OUT_SHIFT;
		DO_LIMIT
	*/
	
	DO_FEEDBACK
	//CH->S0_OUT[1] is in tmp1
	
	//+ SIN_TAB[(in1 >> SIN_LBITS) & SIN_MASK][en1]
	LD_SP	IN1_OFS, tmp0
	LD_SP	EN1_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	add	tmp0, tmp1
	
	//+ SIN_TAB[(in2 >> SIN_LBITS) & SIN_MASK][en2]
	LD_SP	IN2_OFS, tmp0
	LD_SP	EN2_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	add	tmp0, tmp1
	
	//+ SIN_TAB[(in3 >> SIN_LBITS) & SIN_MASK][en3]
	LD_SP	IN3_OFS, tmp0
	LD_SP	EN3_OFS, tmp2
	SIN_TAB	tmp0, tmp2, tmp0
	mov	#-OUT_SHIFT, dstptrptr
	add	tmp0, tmp1
	
	shad	dstptrptr, tmp1
	ST_OFS	tmp1, CHANNEL_OUTd_OFS, channel
	
	DO_LIMIT
.endm

.macro DO_OUTPUT
	//11 cycles
	/*
		buf[0][i] += CH->OUTd & CH->LEFT;
		buf[1][i] += CH->OUTd & CH->RIGHT;
	*/
	//tmp1 should contain CH->OUTd
	LD_SP	LEFT_OFS, tmp2	//LEFT: Load dst ptr
	
	LD_OFS	CHANNEL_LEFT_OFS, channel, tmp0	//LEFT: Load attenuation
	
	mov.l	@tmp2, tmp3	//LEFT: Load existing data
	
	and	tmp1, tmp0	//LEFT: attenuate
	LD_SP	RIGHT_OFS, sintab	//RIGHT: Load dst ptr
	
	LD_OFS	CHANNEL_RIGHT_OFS, channel, r0	//RIGHT: Load attenuation
	add	tmp0, tmp3	//LEFT: Add channel to existing sound
	
	mov.l	tmp3, @tmp2	//LEFT: Update dst buffer
	add	#4, tmp2	//LEFT: Increment dst pointer
	
	mov.l	@sintab, sinmask	//RIGHT: Load existing data
	and	tmp1, r0	//RIGHT: attenuate
	
	ST_SP	tmp2, LEFT_OFS	//LEFT: Update dst pointer
	
	add	r0, sinmask	//RIGHT: Add channel to existing sound
	
	mov.l	sinmask, @sintab	//RIGHT: Update dst buffer
	add	#4, sintab		//RIGHT: Increment dst pointer
	
	ST_SP	sintab, RIGHT_OFS	//RIGHT: Update dst pointer
.endm


.macro S3_END_EARLY_OUT
		mov.l	1f, r0
		mov.w	2f, r1
		mov.l	@(r0, channel), r0
		cmp/eq	r0, r1
		bf	3f
		rts
		 nop
	.align 2
	2:
		.long	ENV_END
	1:
		.word	CHANNEL_SLOT_S3_OFS + SLOT_Ecnt_OFS
	3:
.endm

.macro S1_S3_END_EARLY_OUT
		mov.w	1f, r0
		mov.l	2f, r1
		mov.l	@(r0, channel), r0
		mov.w	4f, slot0
		cmp/eq	r0, r1
		bf	3f
		mov	slot0, r0
		mov.l	@(r0, channel), r0
		cmp/eq	r0, r1
		bf	3f
		rts
		 nop
	.align 2
	2:
		.long	ENV_END
	1:
		.word	CHANNEL_SLOT_S1_OFS + SLOT_Ecnt_OFS
	4:
		.word	CHANNEL_SLOT_S3_OFS + SLOT_Ecnt_OFS
	3:
.endm

.macro S1_S2_S3_END_EARLY_OUT
		mov.l	2f, r1	//Envend
		mov.w	1f, r0	//Slot1
		mov.w	4f, r3	//Slot2
		mov.w	5f, r7	//Slot3
		mov.l	@(r0, channel), r0
		mov	r3, r0
		mov.l	@(r0, channel), r3
		mov	r7, r0
		mov.l	@(r0, channel), r7
		
		cmp/eq	r0, r1
		bf	3f
		cmp/eq	r3, r1
		bf	3f
		cmp/eq	r7, r1
		bf	3f
		
		rts
		 nop
	.align 2
	2:
		.long	ENV_END
	1:
		.word	CHANNEL_SLOT_S1_OFS + SLOT_Ecnt_OFS
	4:
		.word	CHANNEL_SLOT_S2_OFS + SLOT_Ecnt_OFS
	5:
		.word	CHANNEL_SLOT_S3_OFS + SLOT_Ecnt_OFS
	3:
.endm

.macro S0_S1_S2_S3_END_EARLY_OUT
		mov.l	2f, r1	//Envend
		mov.l	6f, r0	//Slot0
		mov.l	1f, r2	//Slot1
		mov.l	4f, r3	//Slot2
		mov.l	5f, r7	//Slot3
		mov.l	@(r0, channel), r0
		mov	r2, r0
		mov.l	@(r0, channel), r2
		mov	r3, r0
		mov.l	@(r0, channel), r3
		mov	r7, r0
		mov.l	@(r0, channel), r7
		
		cmp/eq	r0, r1
		bf	3f
		cmp/eq	r2, r1
		bf	3f
		cmp/eq	r3, r1
		bf	3f
		cmp/eq	r7, r1
		bf	3f
		
		rts
		 nop
	.align 2
	6:
		.long	CHANNEL_SLOT_S0_OFS + SLOT_Ecnt_OFS
	1:
		.long	CHANNEL_SLOT_S1_OFS + SLOT_Ecnt_OFS
	4:
		.long	CHANNEL_SLOT_S2_OFS + SLOT_Ecnt_OFS
	5:
		.long	CHANNEL_SLOT_S3_OFS + SLOT_Ecnt_OFS
	2:
		.long	ENV_END
	3:
.endm


.macro SET_AUDIO_POINTERS
	mov.l	@dstptrptr, tmp0
	mov.l	@(4, dstptrptr), tmp1
	ST_SP	tmp0, LEFT_OFS
	ST_SP	tmp1, RIGHT_OFS
.endm

.macro SET_SLOTS
	mov	channel, slot0
	add	#CHANNEL_SLOT_S0_OFS, slot0
	mov	slot0, slot2
	add	#SLOT_SIZEOF, slot2
	mov	slot2, slot1
	add	#SLOT_SIZEOF, slot1
	mov	slot1, slot3
	add	#SLOT_SIZEOF, slot3
.endm
.macro SET_LFO
	mov.l	1f, tmp0
	mov.l	2f, tmp1
	ST_SP	tmp0, LFO_FREQ_UP_OFS
	bra	3f
	 ST_SP	tmp1, LFO_ENV_UP_OFS
.align 2
1:
	.long	_LFO_FREQ_UP
2:
	.long	_LFO_ENV_UP
3:
.endm
.macro SAVE_REGISTERS_AND_ALLOC_STACK
	mov.l	r8, @-r15
	mov.l	r9, @-r15
	mov.l	r10, @-r15
	mov.l	r11, @-r15
	mov.l	r12, @-r15
	mov.l	r13, @-r15
	mov.l	r14, @-r15
	add	#-SP_ALLOC_SIZE, r15
.endm
.macro RESTORE_REGISTERS_AND_RETURN
	add	#SP_ALLOC_SIZE, r15
	mov.l	@r15+, r14
	mov.l	@r15+, r13
	mov.l	@r15+, r12
	mov.l	@r15+, r11
	mov.l	@r15+, r10
	mov.l	@r15+, r9
	rts
	 mov.l	@r15+, r8
.endm

.globl _Update_Chan_Algo0_Asm
_Update_Chan_Algo0_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA0:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_0
		DO_OUTPUT
		
		dt	length
		bf	.LUCA0
		
		RESTORE_REGISTERS_AND_RETURN
	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE

.globl _Update_Chan_Algo1_Asm
_Update_Chan_Algo1_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA1:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_1
		DO_OUTPUT
		
		dt	length
		bf	.LUCA1
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE

.globl _Update_Chan_Algo2_Asm
_Update_Chan_Algo2_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA2:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_2
		DO_OUTPUT
		
		dt	length
		bf	.LUCA2
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		
.globl _Update_Chan_Algo3_Asm
_Update_Chan_Algo3_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA3:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_3
		DO_OUTPUT
		
		dt	length
		bf	.LUCA3
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE

		
.globl _Update_Chan_Algo4_Asm
_Update_Chan_Algo4_Asm:
		S1_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA4:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_4
		DO_OUTPUT
		
		dt	length
		bf	.LUCA4
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT

.globl _Update_Chan_Algo5_Asm
_Update_Chan_Algo5_Asm:
		S1_S2_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA5:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_5
		DO_OUTPUT
		
		dt	length
		bf	.LUCA5
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT

.globl _Update_Chan_Algo6_Asm
_Update_Chan_Algo6_Asm:
		//Streets of Rage BGM #6
		S1_S2_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA6:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_6
		DO_OUTPUT
		
		dt	length
		bf	.LUCA6
		
		RESTORE_REGISTERS_AND_RETURN
//	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT

.globl _Update_Chan_Algo7_Asm
_Update_Chan_Algo7_Asm:
		//Phantasy Star 2 title music
		S0_S1_S2_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
	
	.LUCA7:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE
		GET_CURRENT_ENV
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_7
		DO_OUTPUT
		
		dt	length
		bf	.LUCA7
		
		RESTORE_REGISTERS_AND_RETURN
//	.align 5
		CONST_ENV_TABLE
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT

.globl _Update_Chan_Algo0_LFO_Asm
_Update_Chan_Algo0_LFO_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA0L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_0
		DO_OUTPUT
		
		dt	length
		bf	.LUCA0L
		
		RESTORE_REGISTERS_AND_RETURN
	.align 5
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE

.globl _Update_Chan_Algo1_LFO_Asm
_Update_Chan_Algo1_LFO_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA1L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_1
		DO_OUTPUT
		
		dt	length
		bf	.LUCA1L
		
		RESTORE_REGISTERS_AND_RETURN
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE

.globl _Update_Chan_Algo2_LFO_Asm
_Update_Chan_Algo2_LFO_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA2L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_2
		DO_OUTPUT
		
		dt	length
		bf	.LUCA2L
		
		RESTORE_REGISTERS_AND_RETURN
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		
.globl _Update_Chan_Algo3_LFO_Asm
_Update_Chan_Algo3_LFO_Asm:
		S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA3L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_3
		DO_OUTPUT
		
		dt	length
		bf	.LUCA3L
		
		RESTORE_REGISTERS_AND_RETURN
//	.align 5
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		
.globl _Update_Chan_Algo4_LFO_Asm
_Update_Chan_Algo4_LFO_Asm:
		S1_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA4L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_4
		DO_OUTPUT
		
		dt	length
		bf	.LUCA4L
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT
////////////////////////////////////////
.globl _Update_Chan_Algo5_LFO_Asm
_Update_Chan_Algo5_LFO_Asm:
		S1_S2_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA5L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_5
		DO_OUTPUT
		
		dt	length
		bf	.LUCA5L
		
		RESTORE_REGISTERS_AND_RETURN
//too far	.align 5
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT

.globl _Update_Chan_Algo6_LFO_Asm
_Update_Chan_Algo6_LFO_Asm:
		//Streets of Rage 2 BGM #6
		S1_S2_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA6L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_6
		DO_OUTPUT
		
		dt	length
		bf	.LUCA6L
		
		RESTORE_REGISTERS_AND_RETURN
//	.align 5
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT

.globl _Update_Chan_Algo7_LFO_Asm
_Update_Chan_Algo7_LFO_Asm:
		//Phantasy Star 2 title music
		S0_S1_S2_S3_END_EARLY_OUT
		
		SAVE_REGISTERS_AND_ALLOC_STACK
		SET_AUDIO_POINTERS
		SET_SLOTS
		SET_LFO
	.LUCA7L:
		GET_CURRENT_PHASE_AND_UPDATE_PHASE_LFO
		GET_CURRENT_ENV_LFO
		UPDATE_ENV
		LOAD_SINTAB_AND_SINMASK
		DO_ALGO_7
		DO_OUTPUT
		
		dt	length
		bf	.LUCA7L
		
		RESTORE_REGISTERS_AND_RETURN
//	.align 5
#ifndef ENABLE_SSG
		CONST_ENV_TABLE
#endif
		CONST_ENV_EVENT
		CONST_SIN_TABLE
		CONST_LIMIT
