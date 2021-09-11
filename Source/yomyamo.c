/**************************************

		�u��ނ���v

						���C��

 **************************************/

#include	"base.h"
#include	"pattern.h"
#include	"sound.h"


enum {	TITLE = 0,					/* �^�C�g�� */
		START,						/* �J�n */
		TURN0,						/* �^�[���O */
		TURN,						/* �^�[�� */
		GET_MOVE,					/* �J�[�h������ */
		SELECT,						/* �J�[�h�I�� */
		PUT_MOVE,					/* �J�[�h��u�� */
		BATTLE0,					/* �퓬�J�n */
		BATTLE,						/* �퓬 */
		ATTACK,						/* �U�� */
		DAMAGE,						/* �̗͌��� */
		EACH,						/* ���ł� */
		GUARD,						/* ���݂��K�[�h */
		RESULT,						/* ���ʕ\�� */
};
static int		stat;				/* ��� */
static Bool		init_flag;			/* �������t���O */
static int		wait_cnt = 0;		/* �҂��J�E���^ */
static int		game_mode = 0;		/* �Q�[�����[�h */

typedef int		Action;				/* �s�� */

enum {	ACT_GUARD, ACT_ATTACK, ACT_THROW = ACT_ATTACK + 6, ACT_TRIP,	/* �s����� */
		ACT_MAX};
enum {	MOT_GUARD, MOT_BEFORE, MOT_AFTER, MOT_DAMAGE, MOT_FALL,			/* ���� */
		MOT_MAX};

#define	ALL_CARD	20				/* �J�[�h���� */
#define	CARD_MAX	 5				/* �莝���̃J�[�h���� */

enum {	MAN = 0, COM};				/* ����� */

/*** �L�����N�^ *******/
typedef struct {
	int		num;				/* �v���C���[�ԍ� */
	int		player;				/* ����� */
	int		hp;					/* �̗� */
	Action	act;				/* �I���s�� */
	Action	prev;				/* �O��̍s�� */
	Action	card[CARD_MAX];		/* �莝���̃J�[�h */
	Action	stock[ALL_CARD];	/* �J�[�h�X�g�b�N */
	int		motion[3][2];		/* ���� */
	Bool	attack;				/* �U���� */
	int		damage;				/* �󂯂��_���[�W */
	int		win;				/* ������ */
} Chr;

/*** �s���p�����[�^ *******/
typedef struct {
	int		power;			/* �З� */
	int		before;			/* �O�s������ */
	int		after;			/* ��s������ */
} Param;

static Chr	chr[2];									/* �L�����N�^ */
static int	time_cnt;								/* �c�莞�ԃJ�E���^ */
static int	turn;									/* �^�[�� */
static int	result;									/* ���� */

const Param		act_param[ACT_MAX] = {
					{-1, 0, 0},						/* �K�[�h */
					{ 5, 1, 1},						/* �W���u */
					{15, 2, 2},						/* �X�g���[�g */
					{25, 2, 4},						/* �A�b�p�[ */
					{25, 3, 2},						/* �񂵏R�� */
					{40, 3, 4},						/* ���яR�� */
					{30, 4, 1},						/* �̓����� */
					{30, 4, 6},						/* ���� */
					{15, 4, 3},						/* ������ */
				};


const int	SELECT_X =  0;							/* �J�[�h�I��`��ʒu */
const int	SELECT_Y = 56;
const int	SELECT_W = 24;
const int	BAR_X    = 55;							/* �����o�[�`��ʒu */
const int	BAR_Y    = 52;
const int	HP_X     =  2;							/* �̗͕`��ʒu */
const int	HP_Y     =  2;
const int	FIELD_X  = 10;							/* ��ԃO���t�B�b�N�`��ʒu */
const int	FIELD_Y  = 12;
const int	BATTLE_X = 36;							/* �o�g�����ʕ\���ʒu */
const int	BATTLE_Y = 12;
const int	MES_Y    = 28;							/* ���b�Z�[�W�`��ʒu */

static int		select_num;							/* �I�𒆂̃J�[�h */
static int		move_cnt;							/* �J�[�h�ړ��A�j���J�E���^ */
static int		move_sx, move_ex, move_sy;			/* �J�[�h�ړ����W */


enum {	SPR_CARD = 0,								/* �X�v���C�g�ԍ� */
		SPR_REVPAT = SPR_CARD + 4, SPR_ACT,
		SPR_BAR = SPR_ACT + ACT_MAX, SPR_HIT = SPR_BAR + 4, SPR_BACK,
		SPR_FONT, SPR_TITLE = SPR_FONT + 26 + 2,
		SPR_READY, SPR_GUARD, SPR_DAMAGE, SPR_FALL,
		SPR_WIN, SPR_DOWN, SPR_ATTACK,
		SPR_MAX = SPR_ATTACK + ACT_MAX - 1};

static Sprite		main_spr[SPR_MAX];				/* ���C���X�v���C�g */
static PIECE_BMP	bmp_sprite0, bmp_sprite1;		/* �X�v���C�g�摜 */
static PIECE_BMP	bmp_pose;						/* �L�����N�^�摜 */
static u_char		vram_buf[SCREEN_W*SCREEN_H];	/* ��ʑޔ��o�b�t�@ */


enum {	SE_MOVE = 0, SE_SELECT,						/* ���ʉ��ԍ� */
		SE_PASS, SE_ATTACK, SE_GUARD,
		SE_MAX};

static PCEWAVEINFO	se_info[SE_MAX];				/* ���ʉ���� */


static int			menu_cnt;						/* ���j���[�p�J�E���^ */
static int			menu_num;						/* ���j���[�I������ */
static u_char		vram_menu[SCREEN_W*SCREEN_H];	/* ���j���[�p��ʑޔ��o�b�t�@ */


#define	save_screen()	memcpy(vram_buf, vram, SCREEN_W*SCREEN_H)		/* ��ʑޔ� */
#define	load_screen()	memcpy(vram, vram_buf, SCREEN_W*SCREEN_H)		/* ��ʕ��A */


/***************************************
    �X�v���C�g�����]�`��
			����	 spr = �X�v���C�g
					x, y = �`����W
 ***************************************/
static
void	draw_sprite_rev(Sprite* spr, int x, int y)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x, y, spr->bmp->header.w - spr->sx - spr->w, spr->sy,
														spr->w, spr->h, spr->param ^ DRW_REVX);
	pceLCDDrawObject(obj);								/* �`�� */
}

/*****************************************
    �X�v���C�g�����`��
			����	   spr = �X�v���C�g
					  x, y = �`����W
					sx, sy = �������W
					  w, h = �傫��
 *****************************************/
static
void	draw_sprite_rgn(Sprite* spr, int x, int y, int sx, int sy, int w, int h)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x, y, spr->sx + sx, spr->sy + sy, w, h, spr->param);
	pceLCDDrawObject(obj);								/* �`�� */
}


/******************
    �^�C�g���`��
 ******************/
static
void	draw_title(void)
{
	const char*		menu[3] = {" VS COM ", " VS MAN ", "  EXIT  "};
	static int		pose0 = -1, pose1 = -1;
	int		i, j;

	for (i = 0; i < SCREEN_H; i += 8) {						/* �w�i�`�� */
		for (j = 0; j < SCREEN_W; j += 8){
			draw_sprite(main_spr + SPR_BACK, j, i);
		}
	}

	if ( (pose0 < 0) || (common_counter % 64 == 0) ) {		/* �|�[�Y�ύX */
		do {
			j = rnd(ACT_MAX - 1);
		} while ( j == pose0 );
		pose0 = j;
	}
	draw_sprite(main_spr + SPR_ATTACK + pose0, 2, 50);
	if ( (pose1 < 0) || (common_counter % 64 == 32) ) {
		do {
			j = rnd(ACT_MAX - 1);
		} while ( j == pose1 );
		pose1 = j;
	}
	draw_sprite_rev(main_spr + SPR_ATTACK + pose1, 94, 50);

	draw_sprite(main_spr + SPR_TITLE, 4, 4);				/* �^�C�g���`�� */
	pceLCDPaint(2, 38, 47, 52, 37);							/* �g�`�� */
	pceLCDPaint(0, 40, 49, 48, 33);
	pceFontSetType(0);										/* ���j���[�`�� */
	for (i = 0; i < 3; i++) {
		j = (i == game_mode) ? 0 : 3;
		pceFontSetTxColor(j);
		pceFontSetBkColor(3 - j);
		pceFontSetPos(44, 51 + i*10);
		pceFontPutStr(menu[i]);
	}
}


/******************
    ���j���[�`��
 ******************/
static
void	draw_menu(void)
{
	const char*		menu[2] = {"YEAH", " NO "};
	int		i, c;

	memcpy(vram, vram_menu, SCREEN_W*SCREEN_H);				/* ��ʕ��A */
	pceLCDPaint(2, 5, 22, 118, 44);							/* �g�`�� */
	pceLCDPaint(0, 7, 24, 114, 40);
	pceFontSetType(0);
	pceFontSetTxColor(3);
	pceFontSetBkColor(0);
	pceFontSetPos(9, 27);
	pceFontPutStr((stat != RESULT) ? "Try again?" : "Keep going?");
	pceFontSetPos(10, 54);									/* �������`�� */
	pceFontPrintf("%d��", chr[0].win);
	pceFontSetPos(93, 54);
	pceFontPrintf("%3d��", chr[1].win);
	for (i = 0; i < 2; i++) {								/* ���ڕ`�� */
		c = (i == menu_num) ? 0 : 3;
		pceFontSetTxColor(c);
		pceFontSetBkColor(3 - c);
		pceFontSetPos(49, 40 + i*11);
		pceFontPutStr(menu[i]);
	}
}

/******************
    ���j���[�I��
 ******************/
static
void	select_menu(void)
{
	Bool	f = FALSE;

	if ( (menu_cnt == 0) || (menu_cnt == -2) ) {			/* ������ */
		menu_cnt = -1;
		menu_num = 0;
		memcpy(vram_menu, vram, SCREEN_W*SCREEN_H);			/* ��ʑޔ� */
		fade_in();
		pceWaveDataOut(1, se_info + SE_PASS);
		f = TRUE;
	}
	if ( (pcePadGet() & TRG_A) && (menu_num == 1) ) {		/* �^�C�g���֖߂� */
		stat = TITLE;
		init_flag = TRUE;
		wait_cnt = 0;
		pceWaveDataOut(1, se_info + SE_SELECT);
		menu_cnt = FADE_COUNT;
		fade_out();											/* ��ʃt�F�[�h�A�E�g */
		fade_sound();										/* �T�E���h�t�F�[�h�A�E�g */
		return;
	}
	if ( pcePadGet() & (TRG_A | TRG_B) ) {					/* �Q�[�����s */
		memcpy(vram, vram_menu, SCREEN_W*SCREEN_H);			/* ��ʕ��A */
		menu_cnt = 0;
		pceWaveDataOut(1, se_info + SE_PASS);
		if ( stat == RESULT ) {
			move_cnt = FADE_COUNT + 1;
		}
		return;
	}
	if ( (pcePadGet() & TRG_UP) && (menu_num > 0) ) {
		menu_num--;
		pceWaveDataOut(1, se_info + SE_MOVE);
		f = TRUE;
	}
	else if ( (pcePadGet() & TRG_DN) && (menu_num < 2 - 1) ) {
		menu_num++;
		pceWaveDataOut(1, se_info + SE_MOVE);
		f = TRUE;
	}
	if ( f ) {
		draw_menu();										/* ���j���[�`�� */
	}
}


/**********************************
    �X�v���C�g���b�Z�[�W�`��
			����	mes = ������
 **********************************/
static
void	draw_message(const char* mes)
{
	int		x = (SCREEN_W - strlen(mes)*12)/2;
	char	c;

	while ( *mes ) {
		if ( (c = *mes++) != ' ' ) {
			draw_sprite(main_spr + ((c < 'A') ? (SPR_FONT + 26 - '1') : (SPR_FONT - 'A')) + c,
																					x, MES_Y);
			x += 12;
		}
		else {
			x += 10;
		}
	}
}


/********************************
    �̗͕`��
		����	p = �L�����N�^
 ********************************/
static
void	draw_hp(Chr* p)
{
	int		x = (p->num == 0) ? HP_X : (SCREEN_W - HP_X - (50 + 2));
	int		t = (p->hp + 1)/2;

	pceLCDPaint(3, x, HP_Y, 50 + 2, 8);
	if ( t > 0 ) {
		pceLCDPaint(1, (p->num == 0) ? (x + 1 + 50 - t) : (x + 1), HP_Y + 1, t, 6);
	}
}

/*********************************
    �J�[�h�`��
		����	 act = �s���ԍ�
				x, y = �`��ʒu
 *********************************/
static
void	draw_card(Action act, int x, int y)
{
	const Param*	p = act_param + act;

	pceLCDPaint(0, x + 1, y + 1, 29, 29);						/* �J�[�h�g */
	pceLCDPaint(3, x + 4, y,      24, 1);
	pceLCDPaint(3, x + 4, y + 30, 24, 1);
	pceLCDPaint(1, x + 4, y + 31, 24, 1);
	pceLCDPaint(3, x,      y + 4, 1, 24);
	pceLCDPaint(3, x + 30, y + 4, 1, 24);
	pceLCDPaint(1, x + 31, y + 4, 1, 24);
	draw_sprite(main_spr + SPR_CARD,     x,      y);
	draw_sprite(main_spr + SPR_CARD + 1, x + 28, y);
	draw_sprite(main_spr + SPR_CARD + 2, x,      y + 28);
	draw_sprite(main_spr + SPR_CARD + 3, x + 28, y + 28);
	if ( act >= 0 ) {
		draw_sprite(main_spr + SPR_ACT + act, x + 2, y + 2);	/* �s���O���t�B�b�N */
		if ( p->power >= 0 ) {									/* �p�����[�^�`�� */
			pceFontSetType(0x82);
			pceFontSetTxColor(3);
			pceFontSetBkColor(0);
			pceFontSetPos(x + 4, y + 22);
			pceFontPrintf("%d/%d %2d", p->before, p->after, p->power);
		}
	}
	else {														/* �J�[�h�� */
		draw_sprite(main_spr + SPR_REVPAT, x + 2, y + 2);
	}
}

/*************************************************
    �����o�[�`��
		����	p = �L�����N�^
				d = ���X�N���[���l�i<0:���s���j
 *************************************************/
static
void	draw_bar(Chr* p, int d)
{
	Sprite*	spr;
	int		x, y, i, j, k, mot;

	x = (p->num == 0) ? BAR_X : (SCREEN_W - BAR_X - 8);
	y = BAR_Y;
	if ( d > 0 ) {											/* �X�N���[�� */
		y += d;
	}
	k = 0;
	for (i = 0; i < 3; i++) {
		if ( (j = p->motion[i][1]) == 0 ) {
			break;
		}
		for (; j > 0; j--) {
			if ( ++k > 14 ) {
				break;
			}
			y -= 4;
			mot = p->motion[i][0];							/* ���� */
			if ( ((d >= 0) && (mot < 0x100))
						|| ((d < 0) && (mot >= 0x100) && (k != (common_counter % 14) + 1)) ) {
				mot &= 0xff;
				if ( mot == MOT_FALL ) {
					mot = MOT_DAMAGE;
				}
				spr = main_spr + SPR_BAR + mot;
				if ( (k == 1) && (d > 0) ) {
					draw_sprite_rgn(spr, x, y, 0, 0, 8, 4 - d);
				}
				else if ( k == 14 ) {
					if ( d > 0 ) {
						draw_sprite_rgn(spr, x, y + 4 - d, 0, 4 - d, 8, d);
					}
				}
				else {
					draw_sprite(spr, x, y);
				}
			}
		}
	}
}

/********************
    �o�g�����ʕ`��
 ********************/
static
void	draw_battle(void)
{
	Sprite*	spr[2];
	int		i, k, num[2];

	pceLCDPaint(2, BATTLE_X + 2, BATTLE_Y + 2, 52, 38);
	pceLCDPaint(1, BATTLE_X + 4, BATTLE_Y + 4, 48, 34);

	k = 0;
	for (i = 0; i < 2; i++) {
		if ( chr[i].motion[0][0] == MOT_GUARD ) {							/* �K�[�h */
			num[k]   = i;
			spr[k++] = main_spr + SPR_GUARD;
		}
		else if ( !chr[i].attack ) {
			if ( chr[i].motion[0][0] == MOT_DAMAGE ) {						/* ���� */
				num[k]   = i;
				spr[k++] = main_spr + SPR_DAMAGE;
			}
			else if ( chr[i].motion[0][0] == MOT_FALL ) {					/* �]�� */
				num[k]   = i;
				spr[k++] = main_spr + SPR_FALL;
			}
		}
	}
	for (i = 0; i < 2; i++) {
		if ( chr[i].attack ) {												/* �U�� */
			num[k]   = i;
			spr[k++] = main_spr + SPR_ATTACK - ACT_ATTACK + chr[i].act;
		}
	}
	for (i = 0; i < 2; i++) {												/* �X�v���C�g�`�� */
		if ( num[i] == 0 ) {
			draw_sprite(spr[i], BATTLE_X + 1, BATTLE_Y + 5);
		}
		else {
			draw_sprite_rev(spr[i], BATTLE_X + 23, BATTLE_Y + 5);
		}
		if ( (chr[num[i]].damage > 0) && !chr[num[i]].attack ) {			/* �q�b�g�}�[�N */
			draw_sprite(main_spr + SPR_HIT, BATTLE_X + 20 + (chr[0].attack ? 6 : 0)
														- (chr[1].attack ? 6 : 0), BATTLE_Y + 10);
		}
	}
	save_screen();
}

/**************
    ��ʕ`��
 **************/
static
void	draw_screen(void)
{
	const char*		result_mes[2][3] = {
						{"WIN", "LOSE", "DRAW"},
						{"1P WIN", "2P WIN", "DRAW"}
					};
	static Chr*		p = chr;
	Sprite*		spr;
	int			i, x;

	for (i = 0; i < SCREEN_H; i += 8) {								/* �w�i�`�� */
		for (x = 0; x < SCREEN_W; x += 8){
			draw_sprite(main_spr + SPR_BACK, x, i);
		}
	}
	for (i = 0; i < 2; i++) {										/* ��ԃO���t�B�b�N */
		x = (i == 0) ? FIELD_X : (SCREEN_W - FIELD_X - 40);
		pceLCDPaint(3, x,     FIELD_Y,     40, 40);
		pceLCDPaint(1, x + 1, FIELD_Y + 1, 38, 38);
		spr = main_spr + ((i == result) ? SPR_WIN : ((chr[i].hp == 0) ? SPR_DOWN
						: ((stat == GUARD) ? SPR_GUARD
						: ((chr[i].motion[0][0] == MOT_DAMAGE) ? SPR_DAMAGE
						: ((chr[i].motion[0][0] == MOT_FALL) ? SPR_FALL
						: ((chr[i].motion[0][0] == MOT_AFTER)
								? (SPR_ATTACK - ACT_ATTACK + chr[i].prev) : SPR_READY))))));
		if ( i == 0 ) {
			draw_sprite(spr, x + 4, FIELD_Y + 4);
		}
		else {
			draw_sprite_rev(spr, x + 4, FIELD_Y + 4);
		}
	}

	draw_hp(chr);													/* �̗͕`�� */
	draw_hp(chr + 1);

	if ( stat == TURN ) {
		p = chr + turn;												/* �J�[�h�`��L�����N�^ */
	}
	for (i = 0; i < CARD_MAX; i++) {								/* �v���C���[�J�[�h�`�� */
		if ( (p->card[i] >= 0) && (i != select_num) ) {
			draw_card(((stat == SELECT) || (stat == TURN)) ? p->card[i] : -1,
															SELECT_X + i*SELECT_W, SELECT_Y);
		}
	}
	if ( select_num >= 0 ) {
		draw_card(p->card[select_num], SELECT_X + select_num*SELECT_W, SELECT_Y - 4);
	}

	if ( (turn > 0) && (stat != GUARD) ) {
		draw_card((turn < 2) ? -1 : chr[0].act, FIELD_X + 4, FIELD_Y + 4);		/* 1P�s�� */
		if ( turn > 1 ) {
			draw_card((turn < 2) ? -1 : chr[1].act, SCREEN_W - FIELD_X - 4 - 32, FIELD_Y + 4);
																				/* 2P�s�� */
		}
	}
	if ( stat == START ) {											/* �J�n */
		draw_message((move_cnt > 64) ? "READY" : "FIGHT");
	}
	else if ( stat == RESULT ) {									/* ���ʕ`�� */
		draw_message(result_mes[(game_mode == 0) ? 0 : 1][result]);
	}
	else if ( stat != BATTLE ) {									/* �����o�[�`�� */
		draw_bar(chr,     0);
		draw_bar(chr + 1, 0);
	}
	save_screen();													/* ��ʑޔ� */
}

/******************************
    �^�[���҂����b�Z�[�W�`��
 ******************************/
static
void	draw_turn(void)
{
	pceLCDPaint(3, 40, 34, 47, 19);
	pceLCDPaint(0, 42, 36, 43, 15);
	pceFontSetType(0);
	pceFontSetTxColor(3);
	pceFontSetBkColor(0);
	pceFontSetPos(46, 39);
	if ( turn < 2 ) {
		pceFontPrintf("%dP TURN", turn + 1);
	}
	else {
		pceFontPutStr("BATTLE!");
	}
}


/********************************
    �s����i�߂�
		����	p = �L�����N�^
 ********************************/
static
void	inc_motion(Chr* p)
{
	p->attack = FALSE;
	if ( --p->motion[0][1] > 0 ) {
		return;
	}
	if ( p->motion[0][0] == MOT_BEFORE ) {				/* �U�� */
		p->attack = TRUE;
		p->prev = p->act;
	}
	p->motion[0][0] = p->motion[1][0];
	p->motion[0][1] = p->motion[1][1];
	p->motion[1][0] = p->motion[2][0];
	p->motion[1][1] = p->motion[2][1];
	p->motion[2][1] = 0;
}


/********************************
    �����ݒ�
		����	p = �L�����N�^
				f = ���肩
 ********************************/
static
void	set_motion(Chr* p, Bool f)
{
	int		t, k = f ? 0 : 0x100;

	t = ((p->motion[0][1] == 0) || (p->motion[0][0] >= 0x100)) ? 0 : 1;
	if ( p->act == ACT_GUARD ) {					/* �K�[�h */
		p->motion[t][0] = MOT_GUARD | k;
		p->motion[t][1] = 100;
	}
	else {											/* �U�� */
		p->motion[t][0] = MOT_BEFORE | k;
		p->motion[t][1] = act_param[p->act].before;
		t++;
		p->motion[t][0] = MOT_AFTER | k;
		p->motion[t][1] = act_param[p->act].after;
	}
	for (t++; t < 3; t++) {
		p->motion[t][1] = 0;
	}
}


/************************************
    �J�[�h������
		����	p = �L�����N�^
		�߂�l	�����J�[�h�����邩
 ************************************/
static
Bool	get_card(Chr* p)
{
	int		i, j;

	for (i = 0; i < CARD_MAX - 1; i++) {					/* �󂫂��l�߂� */
		if ( p->card[i] < 0 ) {
			for (j = i; j < CARD_MAX - 1; j++) {
				p->card[j] = p->card[j + 1];
			}
			p->card[CARD_MAX - 1] = -1;
		}
	}
	if ( p->player == MAN ) {
		draw_screen();										/* ��ʕ`�� */
	}
	if ( (p->card[CARD_MAX - 1] = p->stock[0]) < 0 ) {		/* ��ԏ�̃J�[�h */
		return	FALSE;
	}
	for (i = 0; i < ALL_CARD - 1; i++) {
		p->stock[i] = p->stock[i + 1];
	}
	p->stock[ALL_CARD - 1] = -1;
	return	TRUE;
}

/********************************
    �J�[�h�I��
		����	p = �L�����N�^
		�߂�l	���肵����
 ********************************/
static
Bool	select_card(Chr* p)
{
	Bool	f = FALSE;

	if ( select_num < 0 ) {								/* ������ */
		for (select_num = 0; (select_num < CARD_MAX - 1) && (p->card[select_num + 1] >= 0);
																				select_num++);
		f = TRUE;
	}

	if ( pcePadGet() & TRG_A ) {						/* ���� */
		p->act = p->card[select_num];					/* �I���s�� */
		p->card[select_num] = -1;
		p->motion[(p->motion[0][0] >= 0x100) ? 0 : 1][1] = 0;

		move_sx = SELECT_X + select_num*SELECT_W;		/* �J�[�h�ʒu */
		move_ex = (p->num == 0) ? (FIELD_X + 4) : (SCREEN_W - FIELD_X - 4 - 32);
		move_sy = SELECT_Y;
		return	TRUE;
	}
	if ( pcePadGet() & TRG_LF ) {
		if ( --select_num < 0 ) {
			for (select_num = CARD_MAX - 1; p->card[select_num] < 0; select_num--);
		}
		pceWaveDataOut(1, se_info + SE_MOVE);
		f = TRUE;
	}
	else if ( pcePadGet() & TRG_RI ) {
		select_num++;
		if ( (select_num == CARD_MAX) || (p->card[select_num] < 0) ) {
			select_num = 0;
		}
		pceWaveDataOut(1, se_info + SE_MOVE);
		f = TRUE;
	}
	if ( f ) {
		p->act = p->card[select_num];					/* �I���s�� */
		set_motion(p, FALSE);
		draw_screen();									/* ��ʕ`�� */
	}
	load_screen();
	draw_bar(p, -1);									/* ���̍s���o�[�`�� */
	return	FALSE;
}

/********************************
    �J�[�h�I��COM�v�l
		����	p = �L�����N�^
 ********************************/
static
void	select_card_com(Chr* p)
{
	const int	pr[] = {0, 0, 0, 0, 10, 20, 50, 80, 100, 100, 0, 0};
	Chr*	ep;
	Param const*	pm;
	int		i, t, t_max, d;

	get_card(p);										/* �J�[�h������ */

	ep = chr + (1 - p->num);							/* ���� */
	t_max = -1;
	for (i = 0; (i < CARD_MAX - 1) && (p->card[i] >= 0); i++) {
		d = ep->motion[0][1] - p->motion[0][1];			/* ���̍� */
		switch ( p->card[i] ) {
		  case ACT_GUARD :								/* �K�[�h */
			t = (d > 1) ? 0 : 50;
			break;
		  case ACT_THROW :								/* ���� */
			t = (d < -1) ? 0 : ((d <= 1) ? (d*60 + 100) : 700);
			break;
		  default :										/* �U�� */
			pm = act_param + p->card[i];
			t = pr[d - pm->before + 8]*(pm->before + 1) + (pm->before - 1)*5;
			break;
		}
		if ( t > 0 ) {
			t = rnd(t);
		}
		if ( t > t_max ) {								/* �D��x��r */
			select_num = i;
			t_max = t;
		}
	}

	p->act = p->card[select_num];						/* �I���s�� */
	p->card[select_num] = -1;

	if ( p->num == 0 ) {								/* �J�[�h�ʒu */
		move_sx = FIELD_X + 4 - 32;
		move_ex = FIELD_X + 4;
	}
	else {
		move_sx = SCREEN_W - FIELD_X - 4;
		move_ex = SCREEN_W - FIELD_X - 4 - 32;
	}
	move_sy = -32;
}


/************************************
    �L�����N�^������
		����	p = �L�����N�^
				n = �v���C���[�ԍ�
				c = �����
 ************************************/
static
void	init_chr(Chr* p, int n, int c)
{
	Action	*cp, t;
	int		i, k;

	p->num    = n;								/* �v���C���[�ԍ� */
	p->player = c;								/* ����� */
	p->hp     = 100;							/* �̗� */

	cp = p->stock;								/* �J�[�h�X�g�b�N */
	for (i = 0; i < ACT_MAX; i++) {
		for (k = (i <= ACT_ATTACK) ? 3 : 2; k > 0; k--) {
			*cp++ = (Action)i;
		}
	}
	for (i = 0; i < ALL_CARD; i++) {			/* �V���b�t�� */
		k = rnd(ALL_CARD);
		t = p->stock[i];
		p->stock[i] = p->stock[k];
		p->stock[k] = t;
	}
	cp = p->stock + ALL_CARD - (CARD_MAX - 1);
	for (i = 0; i < CARD_MAX - 1; i++) {		/* �J�[�h��z�� */
		p->card[i] = *cp;
		*cp++ = -1;
	}
	p->card[CARD_MAX - 1] = -1;

	for (i = 0; i < 3; i++) {					/* �����N���A */
		p->motion[i][0] = -1;
		p->motion[i][1] = 0;
	}
}


/************
    ���C��
 ************/
void	app_main(void)
{
	Chr		*p0, *p1;
	int		i, prev;
	Bool	f;

	if ( pcePadGet() & TRG_SELECT ) {				/* �T�E���h ON/OFF */
		pceWaveSetMasterAtt(((sound_flag = !sound_flag) && !snd_fade_cnt) ? master_volume : 127);
	}

	if ( menu_cnt > 0 ) {
		if ( --menu_cnt == 0 ) {					/* �^�C�g���֖߂� */
			stat = TITLE;
			init_flag = TRUE;
		}
		return;
	}
	if ( (menu_cnt < 0) || ((stat != TITLE) && (pcePadGet() & TRG_START)) ) {
		select_menu();								/* ���j���[�I�� */
		return;
	}

	if ( wait_cnt > 0 ) {							/* �҂� */
		wait_cnt--;
		if ( (pcePadGet() & (TRG_A | TRG_B)) && (snd_fade_cnt == 0) ) {
			wait_cnt = 0;
		}
		return;
	}

	prev = stat;
	switch ( stat ) {
	  case TITLE :									/* �^�C�g�� */
		f = FALSE;
		if ( init_flag ) {
			move_cnt = 0;
			fade_in();								/* ��ʃt�F�[�h�C�� */
			play_music(seq_title);					/* �^�C�g��BGM�Đ� */
			f = TRUE;
		}
		if ( move_cnt > 0 ) {
			if ( --move_cnt == 0 ) {
				if ( game_mode < 2 ) {
					chr[0].win = 0;
					chr[1].win = 0;
					stat = START;					/* �Q�[���J�n */
				}
				else {
					pceAppReqExit(0);				/* �I�� */
				}
			}
		}
		else if ( pcePadGet() & TRG_A ) {			/* ���� */
			pceWaveDataOut(1, se_info + SE_SELECT);
			move_cnt = FADE_COUNT;
			fade_out();								/* ��ʃt�F�[�h�A�E�g */
			fade_sound();							/* �T�E���h�t�F�[�h�A�E�g */
		}
		else if ( (pcePadGet() & TRG_UP) && (game_mode > 0) ) {
			game_mode--;
			pceWaveDataOut(1, se_info + SE_MOVE);
			f = TRUE;
		}
		else if ( (pcePadGet() & TRG_DN) && (game_mode < 2) ) {
			game_mode++;
			pceWaveDataOut(1, se_info + SE_MOVE);
			f = TRUE;
		}
		if ( f || (common_counter % 32 == 0) ) {
			draw_title();							/* �^�C�g����ʕ`�� */
		}
		break;

	  case START :									/* �J�n */
		if ( init_flag ) {
			init_chr(chr,     0, (game_mode == 2) ? COM : MAN);		/* �L�����N�^������ */
			init_chr(chr + 1, 1, (game_mode == 1) ? MAN : COM);
			time_cnt = ALL_CARD;					/* �c�莞�� */
			turn = 0;								/* �^�[�� */
			select_num = -1;						/* �I�𒆂̃J�[�h */
			result = -1;							/* ���� */

			move_cnt = 128;
			draw_screen();							/* ��ʕ`�� */
			fade_in();								/* ��ʃt�F�[�h�C�� */
		}
		else if ( --move_cnt == 0 ) {
			stat = TURN0;
		}
		else if ( move_cnt == 64 ) {
			draw_screen();							/* "FIGHT"�`�� */
			play_music(seq_main);					/* ���C��BGM�Đ� */
		}
		break;

	  case TURN0 :									/* �^�[���O */
		if ( turn == 0 ) {
			if ( --time_cnt < 0 ) {
				stat = RESULT;						/* ���Ԑ؂� */
				fade_sound();						/* �T�E���h�t�F�[�h�A�E�g */
				wait_cnt = 32;
				break;
			}
			chr[0].damage = 0;
			chr[1].damage = 0;
			chr[0].attack = FALSE;
			chr[1].attack = FALSE;
		}
		if ( game_mode == 1 ) {						/* �v���C���[��� */
			draw_screen();
			draw_turn();							/* �^�[���҂����b�Z�[�W */
			wait_cnt = 5000000;
		}
		stat = TURN;
		break;

	  case TURN :									/* �^�[�� */
		if ( chr[turn].player == MAN ) {			/* MAN */
			draw_screen();
			stat = get_card(chr + turn) ? GET_MOVE : SELECT;	/* �ꖇ���� */
		}
		else {										/* COM */
			select_card_com(chr + turn);			/* �J�[�h�I�� */
			stat = PUT_MOVE;
		}
		break;

	  case GET_MOVE :								/* �J�[�h�������A�j�� */
		if ( init_flag ) {
			move_cnt = 10;
		}
		load_screen();
		draw_card(-1, SELECT_X + SELECT_W*(CARD_MAX - 1) + move_cnt*3, SELECT_Y);
		if ( --move_cnt < 0 ) {
			stat = SELECT;
			pceWaveDataOut(1, se_info + SE_PASS);
		}
		break;

	  case SELECT :									/* �J�[�h�I�� */
		if ( select_card(chr + turn) ) {
			stat = PUT_MOVE;
		}
		break;

	  case PUT_MOVE :								/* �J�[�h��u���A�j�� */
		if ( init_flag ) {							/* ������ */
			select_num = -1;
			draw_screen();							/* ��ʕ`�� */
			move_cnt = 16;
			pceWaveDataOut(1, se_info + SE_SELECT);
		}
		if ( --move_cnt < 0 ) {
			if ( ++turn < 2 ) {
				stat = TURN0;
			}
			else {
				stat = BATTLE0;
				if ( game_mode == 1 ) {
					draw_turn();
					wait_cnt = 5000000;
				}
			}
			break;
		}
		load_screen();
		draw_card(-1, (move_sx*move_cnt + move_ex*(16 - move_cnt))/16,
										(move_sy*move_cnt + (FIELD_Y + 4)*(16 - move_cnt))/16);
		break;

	  case BATTLE0 :								/* �퓬�J�n */
		set_motion(chr,     TRUE);					/* �����ݒ� */
		set_motion(chr + 1, TRUE);
		draw_screen();
		stat = BATTLE;
		wait_cnt = 50;
		break;

	  case BATTLE :									/* �퓬 */
		if ( init_flag ) {
			draw_screen();
			move_cnt = 0;
		}
		if ( (chr[0].motion[0][0] == MOT_GUARD) && (chr[1].motion[0][0] == MOT_GUARD) ) {
			stat = GUARD;							/* ���݂��K�[�h */
			wait_cnt = 40;
			draw_screen();
			break;
		}
		if ( chr[0].attack || chr[1].attack ) {
			stat = ATTACK;							/* �U�� */
			wait_cnt = 40;
			draw_screen();
			break;
		}
		if ( ++move_cnt == 4*2 ) {
			inc_motion(chr);						/* �s����i�߂� */
			inc_motion(chr + 1);
			move_cnt = 0;
			pceWaveDataOut(1, se_info + SE_PASS);
		}
		load_screen();
		draw_bar(chr,     move_cnt/2);
		draw_bar(chr + 1, move_cnt/2);
		break;

	  case ATTACK :									/* �U�� */
		for (i = 0; i < 2; i++) {
			p0 = chr + i;
			p1 = chr + 1 - i;
			if ( p0->attack ) {
				if ( p0->act == ACT_THROW ) {				/* ���� */
					if ( !p1->attack ) {
						p1->motion[0][0] = MOT_FALL;
						p1->motion[0][1] = 6;
						p1->motion[1][1] = 0;
						p1->damage = act_param[p0->act].power;
					}
					else if ( p1->act != ACT_THROW ) {
						p0->attack = FALSE;
					}
				}
				else if ( p1->motion[0][0] == MOT_GUARD ) {	/* �K�[�h���ꂽ */
					p1->motion[0][1] = 0;
				}
				else {										/* �q�b�g */
					if ( p0->act != ACT_TRIP ) {
						p1->motion[0][0] = MOT_DAMAGE;
						p1->motion[0][1] = 3;
					}
					else {									/* �]�� */
						p1->motion[0][0] = MOT_FALL;
						p1->motion[0][1] = 6;
					}
					p1->motion[1][1] = 0;
					p1->damage = act_param[p0->act].power;
				}
			}
		}
		draw_battle();								/* �o�g�����ʕ`�� */
		if ( chr[0].attack && chr[1].attack ) {
			stat = EACH;
		}
		else {
			stat = DAMAGE;
			pceWaveDataOut(1, ((chr[0].motion[0][0] == MOT_GUARD)
										|| (chr[1].motion[0][0] == MOT_GUARD))
												? (se_info + SE_GUARD) : (se_info + SE_ATTACK));
		}
		wait_cnt = 50;
		break;

	  case EACH :									/* ���ł� */
		chr[0].attack = FALSE;
		chr[1].attack = FALSE;
		if ( chr[0].act != ACT_THROW ) {
			draw_battle();							/* �o�g�����ʕ`�� */
			pceWaveDataOut(1, se_info + SE_ATTACK);
			stat = DAMAGE;
		}
		else {										/* �������m */
			turn = 0;
			stat = TURN0;
		}
		wait_cnt = 50;
		break;

	  case DAMAGE :									/* �̗͌��� */
		load_screen();
		for (i = 0, p0 = chr; i < 2; i++, p0++) {
			if ( p0->damage > 0 ) {
				p0->damage--;
				if ( p0->hp > 0 ) {
					p0->hp--;
				}
			}
			draw_hp(p0);
		}
		if ( (chr[0].damage == 0) && (chr[1].damage == 0) ) {
			turn = 0;
			if ( (chr[0].hp == 0) || (chr[1].hp == 0) ) {
				stat = RESULT;
				fade_sound();						/* �T�E���h�t�F�[�h�A�E�g */
			}
			else {
				stat = TURN0;
			}
			wait_cnt = 150;
		}
		break;

	  case GUARD :									/* ���݂��K�[�h */
		draw_screen();
		chr[0].motion[0][1] = 0;
		chr[1].motion[0][1] = 0;
		chr[0].attack = FALSE;
		chr[1].attack = FALSE;

		turn = 0;
		stat = TURN0;
		wait_cnt = 100;
		break;

	  case RESULT :									/* ���� */
		if ( init_flag ) {
			if ( chr[0].hp > chr[1].hp ) {
				result = 0;
				chr[0].win++;
			}
			else if ( chr[1].hp > chr[0].hp ) {
				result = 1;
				chr[1].win++;
			}
			else {
				result = 2;
			}
			chr[0].motion[0][0] = -1;
			chr[1].motion[0][0] = -1;
			draw_screen();
			play_music(seq_over);					/* �I��BGM�Đ� */
			move_cnt = 400;
		}
		else if ( (move_cnt > FADE_COUNT + 2) && (pcePadGet() & (TRG_A | TRG_B)) ) {
			move_cnt = FADE_COUNT + 2;
		}
		if ( --move_cnt == FADE_COUNT + 1 ) {		/* ���j���[���J�� */
			menu_cnt = -2;
		}
		else if ( move_cnt == FADE_COUNT ) {
			fade_out();								/* ��ʃt�F�[�h�A�E�g */
			fade_sound();							/* �T�E���h�t�F�[�h�A�E�g */
		}
		else if ( move_cnt < 0 ) {
			stat = START;
		}
		break;
	}
	init_flag = (stat != prev);						/* ��ԑJ�� */
}


/************
    ������
 ************/
void	init_app(void)
{
	Sprite*	p;
	int		i;

	set_bmp(&bmp_sprite0, PAT_YOMYAMO0);			/* ���ߐF���� */
	set_bmp(&bmp_sprite1, PAT_YOMYAMO1);			/* ���ߐF���� */
	set_bmp(&bmp_pose, PAT_POSE);					/* �L�����N�^ */
	p = main_spr;
	for (i = 0; i < 4; i++) {										/* �J�[�h�g�̋� */
		set_sprite(p++, &bmp_sprite0, 16 + (i % 2)*4, 42 + i/2*4,  4,  4, DRW_NOMAL);
	}
	set_sprite(p++, &bmp_sprite1, 84,  0, 27, 27, DRW_NOMAL);		/* ���ʖ͗l */
	for (i = 0; i < ACT_MAX; i++) {									/* �s�� */
		set_sprite(p++, &bmp_sprite1, (i % 3)*28, i/3*19, 27, 19, DRW_NOMAL);
	}
	for (i = 0; i < 4; i++) {										/* �s���J�E���^ */
		set_sprite(p++, &bmp_sprite1, 84 + i*8, 28, 8, 4, DRW_NOMAL);
	}
	set_sprite(p++, &bmp_sprite0,  24, 42, 16, 16, DRW_NOMAL);		/* �q�b�g�}�[�N */
	set_sprite(p++, &bmp_sprite1, 112,  0,  8,  8, DRW_NOMAL);		/* �w�i */
	for (i = 0; i < 26 + 2; i++) {									/* �t�H���g */
		set_sprite(p++, &bmp_sprite0, (i % 9)*14, i/9*14, 14, 14, DRW_NOMAL);
	}
	set_sprite(p++, &bmp_sprite1,  0, 57, 120, 40, DRW_NOMAL);		/* �^�C�g�� */

	for (i = 0; i < 5 + ACT_MAX; i++) {								/* ��ԃO���t�B�b�N */
		set_sprite(p++, &bmp_pose, 0, i*32, 32, 32, DRW_NOMAL);
	}

	for (i = 0; i < SE_MAX; i++) {					/* ���ʉ��ݒ� */
		memcpy(se_info + i, se_table[i] + 8, sizeof(PCEWAVEINFO));
		se_info[i].pData = se_table[i] + 8 + sizeof(PCEWAVEINFO);
	}

	stat = TITLE;									/* ��� */
	init_flag = TRUE;
	menu_cnt = 0;
}

/**********
    �I��
 **********/
void	end_app(void)
{
	StopMusic();
	pceWaveStop(0);
}

/**************** End of File ********************************************/