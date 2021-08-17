
#define S0             0  // Stupid typo of the YM2612
#define S1             2
#define S2             1
#define S3             3

#define ATTACK    0
#define DECAY     1
#define SUBSTAIN  2
#define RELEASE   3

#define SLOT_DT_OFS		(0)
#define SLOT_MUL_OFS		(4)
#define SLOT_TL_OFS		(8)
#define SLOT_TLL_OFS		(12)
#define SLOT_SLL_OFS		(16)
#define SLOT_KSR_S_OFS		(20)
#define SLOT_KSR_OFS		(24)
#define SLOT_SEG_OFS		(28)
#define SLOT_PTR_AR_OFS		(32)
#define SLOT_PTR_DR_OFS		(36)
#define SLOT_PTR_SR_OFS		(40)
#define SLOT_PTR_RR_OFS		(44)
#define SLOT_Fcnt_OFS		(48)
#define SLOT_Finc_OFS		(52)
#define SLOT_Ecurp_OFS		(56)
#define SLOT_Ecnt_OFS		(60)
#define SLOT_Einc_OFS		(64)
#define SLOT_Ecmp_OFS		(68)
#define SLOT_EincA_OFS		(72)
#define SLOT_EincD_OFS		(76)
#define SLOT_EincS_OFS		(80)
#define SLOT_EincR_OFS		(84)
#define SLOT_PTR_OUTp_OFS	(88)
#define SLOT_INd_OFS		(92)
#define SLOT_ChgEnM_OFS		(96)
#define SLOT_AMS_OFS		(100)
#define SLOT_AMSon_OFS		(104)
#define SLOT_SIZEOF		(108)

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
#define CHANNEL_FFlag;		(528) /*[4]  Frequency step recalculation flag*/
#define CHANNEL_SIZEOF;		(532) /*[4]  Frequency step recalculation flag*/


#define tmp0		r1
#define tmp1		r2
#define tmp2		r3
#define channel		r4
#define dstptrptr	r5
#define length		r6
#define in0		r7
#define in1		r8
#define in2		r9
#define in3		r10
#define en0		r11
#define en1		r12
#define en2		r13
#define en3		r14

#define SP_BASE	(0)
#define IN0_OFS	(SP_BASE + 0)
#define IN1_OFS	(SP_BASE + 4)
#define IN2_OFS	(SP_BASE + 8)
#define IN3_OFS	(SP_BASE + 12)
#define EN0_OFS	(SP_BASE + 16)
#define EN1_OFS	(SP_BASE + 20)
#define EN2_OFS	(SP_BASE + 24)
#define EN3_OFS	(SP_BASE + 28)



.macro GET_CURRENT_PHASE
	/*
		in0 = CH->SLOT[S0].Fcnt;
		in1 = CH->SLOT[S1].Fcnt;
		in2 = CH->SLOT[S2].Fcnt;
		in3 = CH->SLOT[S3].Fcnt;
	*/
	mov	channel, tmp0
	add	#CHANNEL_SLOT_S0_OFS + SLOT_Fcnt_OFS, tmp0
	
	//S0
	mov.l	@tmp0, in0
	add	#SLOT_SIZEOF, tmp0
	
	//S2
	mov.l	@tmp0, in2
	add	#SLOT_SIZEOF, tmp0
	
	//S1
	mov.l	@tmp0, in1
	add	#SLOT_SIZEOF, tmp0
	
	//S3
	mov.l	@tmp0, in3
	add	#SLOT_SIZEOF, tmp0
.endm

.macro UPDATE_PHASE
	/*
		CH->SLOT[S0].Fcnt += CH->SLOT[S0].Finc;
		CH->SLOT[S1].Fcnt += CH->SLOT[S1].Finc;
		CH->SLOT[S2].Fcnt += CH->SLOT[S2].Finc;
		CH->SLOT[S3].Fcnt += CH->SLOT[S3].Finc;
	*?
	mov	channel, tmp0
	add	#CHANNEL_SLOT, tmp0
	
	//S0
	mov.l @(tmp0, SLOT_Fcnt_OFS), tmp1
	mov.l @(tmp0, SLOT_Finc_OFS), tmp2
	add	tmp1, tmp2
	mov.l tmp2, @(tmp0, SLOT_Fcnt_OFS)
	add	#SLOT_SIZEOF, tmp0
	
	//S2
	mov.l @(tmp0, SLOT_Fcnt_OFS), tmp1
	mov.l @(tmp0, SLOT_Finc_OFS), tmp2
	add	tmp1, tmp2
	mov.l tmp2, @(tmp0, SLOT_Fcnt_OFS)
	add	#SLOT_SIZEOF, tmp0
	
	//S1
	mov.l @(tmp0, SLOT_Fcnt_OFS), tmp1
	mov.l @(tmp0, SLOT_Finc_OFS), tmp2
	add	tmp1, tmp2
	mov.l tmp2, @(tmp0, SLOT_Fcnt_OFS)
	add	#SLOT_SIZEOF, tmp0
	
	//S3
	mov.l @(tmp0, SLOT_Fcnt_OFS), tmp1
	mov.l @(tmp0, SLOT_Finc_OFS), tmp2
	add	tmp1, tmp2
	mov.l tmp2, @(tmp0, SLOT_Fcnt_OFS)
.endm
