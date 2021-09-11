/**************************************

		「よむやも」

						メイン

 **************************************/

#include	"base.h"
#include	"pattern.h"
#include	"sound.h"


enum {	TITLE = 0,					/* タイトル */
		START,						/* 開始 */
		TURN0,						/* ターン前 */
		TURN,						/* ターン */
		GET_MOVE,					/* カードを引く */
		SELECT,						/* カード選択 */
		PUT_MOVE,					/* カードを置く */
		BATTLE0,					/* 戦闘開始 */
		BATTLE,						/* 戦闘 */
		ATTACK,						/* 攻撃 */
		DAMAGE,						/* 体力減少 */
		EACH,						/* 相打ち */
		GUARD,						/* お互いガード */
		RESULT,						/* 結果表示 */
};
static int		stat;				/* 状態 */
static Bool		init_flag;			/* 初期化フラグ */
static int		wait_cnt = 0;		/* 待ちカウンタ */
static int		game_mode = 0;		/* ゲームモード */

typedef int		Action;				/* 行動 */

enum {	ACT_GUARD, ACT_ATTACK, ACT_THROW = ACT_ATTACK + 6, ACT_TRIP,	/* 行動種類 */
		ACT_MAX};
enum {	MOT_GUARD, MOT_BEFORE, MOT_AFTER, MOT_DAMAGE, MOT_FALL,			/* 動き */
		MOT_MAX};

#define	ALL_CARD	20				/* カード総数 */
#define	CARD_MAX	 5				/* 手持ちのカード枚数 */

enum {	MAN = 0, COM};				/* 操作主 */

/*** キャラクタ *******/
typedef struct {
	int		num;				/* プレイヤー番号 */
	int		player;				/* 操作主 */
	int		hp;					/* 体力 */
	Action	act;				/* 選択行動 */
	Action	prev;				/* 前回の行動 */
	Action	card[CARD_MAX];		/* 手持ちのカード */
	Action	stock[ALL_CARD];	/* カードストック */
	int		motion[3][2];		/* 動き */
	Bool	attack;				/* 攻撃か */
	int		damage;				/* 受けたダメージ */
	int		win;				/* 勝利数 */
} Chr;

/*** 行動パラメータ *******/
typedef struct {
	int		power;			/* 威力 */
	int		before;			/* 前行動時間 */
	int		after;			/* 後行動時間 */
} Param;

static Chr	chr[2];									/* キャラクタ */
static int	time_cnt;								/* 残り時間カウンタ */
static int	turn;									/* ターン */
static int	result;									/* 結果 */

const Param		act_param[ACT_MAX] = {
					{-1, 0, 0},						/* ガード */
					{ 5, 1, 1},						/* ジャブ */
					{15, 2, 2},						/* ストレート */
					{25, 2, 4},						/* アッパー */
					{25, 3, 2},						/* 回し蹴り */
					{40, 3, 4},						/* 跳び蹴り */
					{30, 4, 1},						/* 体当たり */
					{30, 4, 6},						/* 投げ */
					{15, 4, 3},						/* 足払い */
				};


const int	SELECT_X =  0;							/* カード選択描画位置 */
const int	SELECT_Y = 56;
const int	SELECT_W = 24;
const int	BAR_X    = 55;							/* 動きバー描画位置 */
const int	BAR_Y    = 52;
const int	HP_X     =  2;							/* 体力描画位置 */
const int	HP_Y     =  2;
const int	FIELD_X  = 10;							/* 状態グラフィック描画位置 */
const int	FIELD_Y  = 12;
const int	BATTLE_X = 36;							/* バトル結果表示位置 */
const int	BATTLE_Y = 12;
const int	MES_Y    = 28;							/* メッセージ描画位置 */

static int		select_num;							/* 選択中のカード */
static int		move_cnt;							/* カード移動アニメカウンタ */
static int		move_sx, move_ex, move_sy;			/* カード移動座標 */


enum {	SPR_CARD = 0,								/* スプライト番号 */
		SPR_REVPAT = SPR_CARD + 4, SPR_ACT,
		SPR_BAR = SPR_ACT + ACT_MAX, SPR_HIT = SPR_BAR + 4, SPR_BACK,
		SPR_FONT, SPR_TITLE = SPR_FONT + 26 + 2,
		SPR_READY, SPR_GUARD, SPR_DAMAGE, SPR_FALL,
		SPR_WIN, SPR_DOWN, SPR_ATTACK,
		SPR_MAX = SPR_ATTACK + ACT_MAX - 1};

static Sprite		main_spr[SPR_MAX];				/* メインスプライト */
static PIECE_BMP	bmp_sprite0, bmp_sprite1;		/* スプライト画像 */
static PIECE_BMP	bmp_pose;						/* キャラクタ画像 */
static u_char		vram_buf[SCREEN_W*SCREEN_H];	/* 画面退避バッファ */


enum {	SE_MOVE = 0, SE_SELECT,						/* 効果音番号 */
		SE_PASS, SE_ATTACK, SE_GUARD,
		SE_MAX};

static PCEWAVEINFO	se_info[SE_MAX];				/* 効果音情報 */


static int			menu_cnt;						/* メニュー用カウンタ */
static int			menu_num;						/* メニュー選択項目 */
static u_char		vram_menu[SCREEN_W*SCREEN_H];	/* メニュー用画面退避バッファ */


#define	save_screen()	memcpy(vram_buf, vram, SCREEN_W*SCREEN_H)		/* 画面退避 */
#define	load_screen()	memcpy(vram, vram_buf, SCREEN_W*SCREEN_H)		/* 画面復帰 */


/***************************************
    スプライト横反転描画
			引数	 spr = スプライト
					x, y = 描画座標
 ***************************************/
static
void	draw_sprite_rev(Sprite* spr, int x, int y)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x, y, spr->bmp->header.w - spr->sx - spr->w, spr->sy,
														spr->w, spr->h, spr->param ^ DRW_REVX);
	pceLCDDrawObject(obj);								/* 描画 */
}

/*****************************************
    スプライト部分描画
			引数	   spr = スプライト
					  x, y = 描画座標
					sx, sy = 部分座標
					  w, h = 大きさ
 *****************************************/
static
void	draw_sprite_rgn(Sprite* spr, int x, int y, int sx, int sy, int w, int h)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x, y, spr->sx + sx, spr->sy + sy, w, h, spr->param);
	pceLCDDrawObject(obj);								/* 描画 */
}


/******************
    タイトル描画
 ******************/
static
void	draw_title(void)
{
	const char*		menu[3] = {" VS COM ", " VS MAN ", "  EXIT  "};
	static int		pose0 = -1, pose1 = -1;
	int		i, j;

	for (i = 0; i < SCREEN_H; i += 8) {						/* 背景描画 */
		for (j = 0; j < SCREEN_W; j += 8){
			draw_sprite(main_spr + SPR_BACK, j, i);
		}
	}

	if ( (pose0 < 0) || (common_counter % 64 == 0) ) {		/* ポーズ変更 */
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

	draw_sprite(main_spr + SPR_TITLE, 4, 4);				/* タイトル描画 */
	pceLCDPaint(2, 38, 47, 52, 37);							/* 枠描画 */
	pceLCDPaint(0, 40, 49, 48, 33);
	pceFontSetType(0);										/* メニュー描画 */
	for (i = 0; i < 3; i++) {
		j = (i == game_mode) ? 0 : 3;
		pceFontSetTxColor(j);
		pceFontSetBkColor(3 - j);
		pceFontSetPos(44, 51 + i*10);
		pceFontPutStr(menu[i]);
	}
}


/******************
    メニュー描画
 ******************/
static
void	draw_menu(void)
{
	const char*		menu[2] = {"YEAH", " NO "};
	int		i, c;

	memcpy(vram, vram_menu, SCREEN_W*SCREEN_H);				/* 画面復帰 */
	pceLCDPaint(2, 5, 22, 118, 44);							/* 枠描画 */
	pceLCDPaint(0, 7, 24, 114, 40);
	pceFontSetType(0);
	pceFontSetTxColor(3);
	pceFontSetBkColor(0);
	pceFontSetPos(9, 27);
	pceFontPutStr((stat != RESULT) ? "Try again?" : "Keep going?");
	pceFontSetPos(10, 54);									/* 勝利数描画 */
	pceFontPrintf("%d勝", chr[0].win);
	pceFontSetPos(93, 54);
	pceFontPrintf("%3d勝", chr[1].win);
	for (i = 0; i < 2; i++) {								/* 項目描画 */
		c = (i == menu_num) ? 0 : 3;
		pceFontSetTxColor(c);
		pceFontSetBkColor(3 - c);
		pceFontSetPos(49, 40 + i*11);
		pceFontPutStr(menu[i]);
	}
}

/******************
    メニュー選択
 ******************/
static
void	select_menu(void)
{
	Bool	f = FALSE;

	if ( (menu_cnt == 0) || (menu_cnt == -2) ) {			/* 初期化 */
		menu_cnt = -1;
		menu_num = 0;
		memcpy(vram_menu, vram, SCREEN_W*SCREEN_H);			/* 画面退避 */
		fade_in();
		pceWaveDataOut(1, se_info + SE_PASS);
		f = TRUE;
	}
	if ( (pcePadGet() & TRG_A) && (menu_num == 1) ) {		/* タイトルへ戻る */
		stat = TITLE;
		init_flag = TRUE;
		wait_cnt = 0;
		pceWaveDataOut(1, se_info + SE_SELECT);
		menu_cnt = FADE_COUNT;
		fade_out();											/* 画面フェードアウト */
		fade_sound();										/* サウンドフェードアウト */
		return;
	}
	if ( pcePadGet() & (TRG_A | TRG_B) ) {					/* ゲーム続行 */
		memcpy(vram, vram_menu, SCREEN_W*SCREEN_H);			/* 画面復帰 */
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
		draw_menu();										/* メニュー描画 */
	}
}


/**********************************
    スプライトメッセージ描画
			引数	mes = 文字列
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
    体力描画
		引数	p = キャラクタ
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
    カード描画
		引数	 act = 行動番号
				x, y = 描画位置
 *********************************/
static
void	draw_card(Action act, int x, int y)
{
	const Param*	p = act_param + act;

	pceLCDPaint(0, x + 1, y + 1, 29, 29);						/* カード枠 */
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
		draw_sprite(main_spr + SPR_ACT + act, x + 2, y + 2);	/* 行動グラフィック */
		if ( p->power >= 0 ) {									/* パラメータ描画 */
			pceFontSetType(0x82);
			pceFontSetTxColor(3);
			pceFontSetBkColor(0);
			pceFontSetPos(x + 4, y + 22);
			pceFontPrintf("%d/%d %2d", p->before, p->after, p->power);
		}
	}
	else {														/* カード裏 */
		draw_sprite(main_spr + SPR_REVPAT, x + 2, y + 2);
	}
}

/*************************************************
    動きバー描画
		引数	p = キャラクタ
				d = 下スクロール値（<0:仮行動）
 *************************************************/
static
void	draw_bar(Chr* p, int d)
{
	Sprite*	spr;
	int		x, y, i, j, k, mot;

	x = (p->num == 0) ? BAR_X : (SCREEN_W - BAR_X - 8);
	y = BAR_Y;
	if ( d > 0 ) {											/* スクロール */
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
			mot = p->motion[i][0];							/* 動き */
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
    バトル結果描画
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
		if ( chr[i].motion[0][0] == MOT_GUARD ) {							/* ガード */
			num[k]   = i;
			spr[k++] = main_spr + SPR_GUARD;
		}
		else if ( !chr[i].attack ) {
			if ( chr[i].motion[0][0] == MOT_DAMAGE ) {						/* やられ */
				num[k]   = i;
				spr[k++] = main_spr + SPR_DAMAGE;
			}
			else if ( chr[i].motion[0][0] == MOT_FALL ) {					/* 転び */
				num[k]   = i;
				spr[k++] = main_spr + SPR_FALL;
			}
		}
	}
	for (i = 0; i < 2; i++) {
		if ( chr[i].attack ) {												/* 攻撃 */
			num[k]   = i;
			spr[k++] = main_spr + SPR_ATTACK - ACT_ATTACK + chr[i].act;
		}
	}
	for (i = 0; i < 2; i++) {												/* スプライト描画 */
		if ( num[i] == 0 ) {
			draw_sprite(spr[i], BATTLE_X + 1, BATTLE_Y + 5);
		}
		else {
			draw_sprite_rev(spr[i], BATTLE_X + 23, BATTLE_Y + 5);
		}
		if ( (chr[num[i]].damage > 0) && !chr[num[i]].attack ) {			/* ヒットマーク */
			draw_sprite(main_spr + SPR_HIT, BATTLE_X + 20 + (chr[0].attack ? 6 : 0)
														- (chr[1].attack ? 6 : 0), BATTLE_Y + 10);
		}
	}
	save_screen();
}

/**************
    画面描画
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

	for (i = 0; i < SCREEN_H; i += 8) {								/* 背景描画 */
		for (x = 0; x < SCREEN_W; x += 8){
			draw_sprite(main_spr + SPR_BACK, x, i);
		}
	}
	for (i = 0; i < 2; i++) {										/* 状態グラフィック */
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

	draw_hp(chr);													/* 体力描画 */
	draw_hp(chr + 1);

	if ( stat == TURN ) {
		p = chr + turn;												/* カード描画キャラクタ */
	}
	for (i = 0; i < CARD_MAX; i++) {								/* プレイヤーカード描画 */
		if ( (p->card[i] >= 0) && (i != select_num) ) {
			draw_card(((stat == SELECT) || (stat == TURN)) ? p->card[i] : -1,
															SELECT_X + i*SELECT_W, SELECT_Y);
		}
	}
	if ( select_num >= 0 ) {
		draw_card(p->card[select_num], SELECT_X + select_num*SELECT_W, SELECT_Y - 4);
	}

	if ( (turn > 0) && (stat != GUARD) ) {
		draw_card((turn < 2) ? -1 : chr[0].act, FIELD_X + 4, FIELD_Y + 4);		/* 1P行動 */
		if ( turn > 1 ) {
			draw_card((turn < 2) ? -1 : chr[1].act, SCREEN_W - FIELD_X - 4 - 32, FIELD_Y + 4);
																				/* 2P行動 */
		}
	}
	if ( stat == START ) {											/* 開始 */
		draw_message((move_cnt > 64) ? "READY" : "FIGHT");
	}
	else if ( stat == RESULT ) {									/* 結果描画 */
		draw_message(result_mes[(game_mode == 0) ? 0 : 1][result]);
	}
	else if ( stat != BATTLE ) {									/* 動きバー描画 */
		draw_bar(chr,     0);
		draw_bar(chr + 1, 0);
	}
	save_screen();													/* 画面退避 */
}

/******************************
    ターン待ちメッセージ描画
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
    行動を進める
		引数	p = キャラクタ
 ********************************/
static
void	inc_motion(Chr* p)
{
	p->attack = FALSE;
	if ( --p->motion[0][1] > 0 ) {
		return;
	}
	if ( p->motion[0][0] == MOT_BEFORE ) {				/* 攻撃 */
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
    動き設定
		引数	p = キャラクタ
				f = 決定か
 ********************************/
static
void	set_motion(Chr* p, Bool f)
{
	int		t, k = f ? 0 : 0x100;

	t = ((p->motion[0][1] == 0) || (p->motion[0][0] >= 0x100)) ? 0 : 1;
	if ( p->act == ACT_GUARD ) {					/* ガード */
		p->motion[t][0] = MOT_GUARD | k;
		p->motion[t][1] = 100;
	}
	else {											/* 攻撃 */
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
    カードを引く
		引数	p = キャラクタ
		戻り値	引くカードがあるか
 ************************************/
static
Bool	get_card(Chr* p)
{
	int		i, j;

	for (i = 0; i < CARD_MAX - 1; i++) {					/* 空きを詰める */
		if ( p->card[i] < 0 ) {
			for (j = i; j < CARD_MAX - 1; j++) {
				p->card[j] = p->card[j + 1];
			}
			p->card[CARD_MAX - 1] = -1;
		}
	}
	if ( p->player == MAN ) {
		draw_screen();										/* 画面描画 */
	}
	if ( (p->card[CARD_MAX - 1] = p->stock[0]) < 0 ) {		/* 一番上のカード */
		return	FALSE;
	}
	for (i = 0; i < ALL_CARD - 1; i++) {
		p->stock[i] = p->stock[i + 1];
	}
	p->stock[ALL_CARD - 1] = -1;
	return	TRUE;
}

/********************************
    カード選択
		引数	p = キャラクタ
		戻り値	決定したか
 ********************************/
static
Bool	select_card(Chr* p)
{
	Bool	f = FALSE;

	if ( select_num < 0 ) {								/* 初期化 */
		for (select_num = 0; (select_num < CARD_MAX - 1) && (p->card[select_num + 1] >= 0);
																				select_num++);
		f = TRUE;
	}

	if ( pcePadGet() & TRG_A ) {						/* 決定 */
		p->act = p->card[select_num];					/* 選択行動 */
		p->card[select_num] = -1;
		p->motion[(p->motion[0][0] >= 0x100) ? 0 : 1][1] = 0;

		move_sx = SELECT_X + select_num*SELECT_W;		/* カード位置 */
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
		p->act = p->card[select_num];					/* 選択行動 */
		set_motion(p, FALSE);
		draw_screen();									/* 画面描画 */
	}
	load_screen();
	draw_bar(p, -1);									/* 仮の行動バー描画 */
	return	FALSE;
}

/********************************
    カード選択COM思考
		引数	p = キャラクタ
 ********************************/
static
void	select_card_com(Chr* p)
{
	const int	pr[] = {0, 0, 0, 0, 10, 20, 50, 80, 100, 100, 0, 0};
	Chr*	ep;
	Param const*	pm;
	int		i, t, t_max, d;

	get_card(p);										/* カードを引く */

	ep = chr + (1 - p->num);							/* 相手 */
	t_max = -1;
	for (i = 0; (i < CARD_MAX - 1) && (p->card[i] >= 0); i++) {
		d = ep->motion[0][1] - p->motion[0][1];			/* 隙の差 */
		switch ( p->card[i] ) {
		  case ACT_GUARD :								/* ガード */
			t = (d > 1) ? 0 : 50;
			break;
		  case ACT_THROW :								/* 投げ */
			t = (d < -1) ? 0 : ((d <= 1) ? (d*60 + 100) : 700);
			break;
		  default :										/* 攻撃 */
			pm = act_param + p->card[i];
			t = pr[d - pm->before + 8]*(pm->before + 1) + (pm->before - 1)*5;
			break;
		}
		if ( t > 0 ) {
			t = rnd(t);
		}
		if ( t > t_max ) {								/* 優先度比較 */
			select_num = i;
			t_max = t;
		}
	}

	p->act = p->card[select_num];						/* 選択行動 */
	p->card[select_num] = -1;

	if ( p->num == 0 ) {								/* カード位置 */
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
    キャラクタ初期化
		引数	p = キャラクタ
				n = プレイヤー番号
				c = 操作主
 ************************************/
static
void	init_chr(Chr* p, int n, int c)
{
	Action	*cp, t;
	int		i, k;

	p->num    = n;								/* プレイヤー番号 */
	p->player = c;								/* 操作主 */
	p->hp     = 100;							/* 体力 */

	cp = p->stock;								/* カードストック */
	for (i = 0; i < ACT_MAX; i++) {
		for (k = (i <= ACT_ATTACK) ? 3 : 2; k > 0; k--) {
			*cp++ = (Action)i;
		}
	}
	for (i = 0; i < ALL_CARD; i++) {			/* シャッフル */
		k = rnd(ALL_CARD);
		t = p->stock[i];
		p->stock[i] = p->stock[k];
		p->stock[k] = t;
	}
	cp = p->stock + ALL_CARD - (CARD_MAX - 1);
	for (i = 0; i < CARD_MAX - 1; i++) {		/* カードを配る */
		p->card[i] = *cp;
		*cp++ = -1;
	}
	p->card[CARD_MAX - 1] = -1;

	for (i = 0; i < 3; i++) {					/* 動きクリア */
		p->motion[i][0] = -1;
		p->motion[i][1] = 0;
	}
}


/************
    メイン
 ************/
void	app_main(void)
{
	Chr		*p0, *p1;
	int		i, prev;
	Bool	f;

	if ( pcePadGet() & TRG_SELECT ) {				/* サウンド ON/OFF */
		pceWaveSetMasterAtt(((sound_flag = !sound_flag) && !snd_fade_cnt) ? master_volume : 127);
	}

	if ( menu_cnt > 0 ) {
		if ( --menu_cnt == 0 ) {					/* タイトルへ戻る */
			stat = TITLE;
			init_flag = TRUE;
		}
		return;
	}
	if ( (menu_cnt < 0) || ((stat != TITLE) && (pcePadGet() & TRG_START)) ) {
		select_menu();								/* メニュー選択 */
		return;
	}

	if ( wait_cnt > 0 ) {							/* 待ち */
		wait_cnt--;
		if ( (pcePadGet() & (TRG_A | TRG_B)) && (snd_fade_cnt == 0) ) {
			wait_cnt = 0;
		}
		return;
	}

	prev = stat;
	switch ( stat ) {
	  case TITLE :									/* タイトル */
		f = FALSE;
		if ( init_flag ) {
			move_cnt = 0;
			fade_in();								/* 画面フェードイン */
			play_music(seq_title);					/* タイトルBGM再生 */
			f = TRUE;
		}
		if ( move_cnt > 0 ) {
			if ( --move_cnt == 0 ) {
				if ( game_mode < 2 ) {
					chr[0].win = 0;
					chr[1].win = 0;
					stat = START;					/* ゲーム開始 */
				}
				else {
					pceAppReqExit(0);				/* 終了 */
				}
			}
		}
		else if ( pcePadGet() & TRG_A ) {			/* 決定 */
			pceWaveDataOut(1, se_info + SE_SELECT);
			move_cnt = FADE_COUNT;
			fade_out();								/* 画面フェードアウト */
			fade_sound();							/* サウンドフェードアウト */
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
			draw_title();							/* タイトル画面描画 */
		}
		break;

	  case START :									/* 開始 */
		if ( init_flag ) {
			init_chr(chr,     0, (game_mode == 2) ? COM : MAN);		/* キャラクタ初期化 */
			init_chr(chr + 1, 1, (game_mode == 1) ? MAN : COM);
			time_cnt = ALL_CARD;					/* 残り時間 */
			turn = 0;								/* ターン */
			select_num = -1;						/* 選択中のカード */
			result = -1;							/* 結果 */

			move_cnt = 128;
			draw_screen();							/* 画面描画 */
			fade_in();								/* 画面フェードイン */
		}
		else if ( --move_cnt == 0 ) {
			stat = TURN0;
		}
		else if ( move_cnt == 64 ) {
			draw_screen();							/* "FIGHT"描画 */
			play_music(seq_main);					/* メインBGM再生 */
		}
		break;

	  case TURN0 :									/* ターン前 */
		if ( turn == 0 ) {
			if ( --time_cnt < 0 ) {
				stat = RESULT;						/* 時間切れ */
				fade_sound();						/* サウンドフェードアウト */
				wait_cnt = 32;
				break;
			}
			chr[0].damage = 0;
			chr[1].damage = 0;
			chr[0].attack = FALSE;
			chr[1].attack = FALSE;
		}
		if ( game_mode == 1 ) {						/* プレイヤー交替 */
			draw_screen();
			draw_turn();							/* ターン待ちメッセージ */
			wait_cnt = 5000000;
		}
		stat = TURN;
		break;

	  case TURN :									/* ターン */
		if ( chr[turn].player == MAN ) {			/* MAN */
			draw_screen();
			stat = get_card(chr + turn) ? GET_MOVE : SELECT;	/* 一枚引く */
		}
		else {										/* COM */
			select_card_com(chr + turn);			/* カード選択 */
			stat = PUT_MOVE;
		}
		break;

	  case GET_MOVE :								/* カードを引くアニメ */
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

	  case SELECT :									/* カード選択中 */
		if ( select_card(chr + turn) ) {
			stat = PUT_MOVE;
		}
		break;

	  case PUT_MOVE :								/* カードを置くアニメ */
		if ( init_flag ) {							/* 初期化 */
			select_num = -1;
			draw_screen();							/* 画面描画 */
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

	  case BATTLE0 :								/* 戦闘開始 */
		set_motion(chr,     TRUE);					/* 動き設定 */
		set_motion(chr + 1, TRUE);
		draw_screen();
		stat = BATTLE;
		wait_cnt = 50;
		break;

	  case BATTLE :									/* 戦闘 */
		if ( init_flag ) {
			draw_screen();
			move_cnt = 0;
		}
		if ( (chr[0].motion[0][0] == MOT_GUARD) && (chr[1].motion[0][0] == MOT_GUARD) ) {
			stat = GUARD;							/* お互いガード */
			wait_cnt = 40;
			draw_screen();
			break;
		}
		if ( chr[0].attack || chr[1].attack ) {
			stat = ATTACK;							/* 攻撃 */
			wait_cnt = 40;
			draw_screen();
			break;
		}
		if ( ++move_cnt == 4*2 ) {
			inc_motion(chr);						/* 行動を進める */
			inc_motion(chr + 1);
			move_cnt = 0;
			pceWaveDataOut(1, se_info + SE_PASS);
		}
		load_screen();
		draw_bar(chr,     move_cnt/2);
		draw_bar(chr + 1, move_cnt/2);
		break;

	  case ATTACK :									/* 攻撃 */
		for (i = 0; i < 2; i++) {
			p0 = chr + i;
			p1 = chr + 1 - i;
			if ( p0->attack ) {
				if ( p0->act == ACT_THROW ) {				/* 投げ */
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
				else if ( p1->motion[0][0] == MOT_GUARD ) {	/* ガードされた */
					p1->motion[0][1] = 0;
				}
				else {										/* ヒット */
					if ( p0->act != ACT_TRIP ) {
						p1->motion[0][0] = MOT_DAMAGE;
						p1->motion[0][1] = 3;
					}
					else {									/* 転ぶ */
						p1->motion[0][0] = MOT_FALL;
						p1->motion[0][1] = 6;
					}
					p1->motion[1][1] = 0;
					p1->damage = act_param[p0->act].power;
				}
			}
		}
		draw_battle();								/* バトル結果描画 */
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

	  case EACH :									/* 相打ち */
		chr[0].attack = FALSE;
		chr[1].attack = FALSE;
		if ( chr[0].act != ACT_THROW ) {
			draw_battle();							/* バトル結果描画 */
			pceWaveDataOut(1, se_info + SE_ATTACK);
			stat = DAMAGE;
		}
		else {										/* 投げ同士 */
			turn = 0;
			stat = TURN0;
		}
		wait_cnt = 50;
		break;

	  case DAMAGE :									/* 体力減少 */
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
				fade_sound();						/* サウンドフェードアウト */
			}
			else {
				stat = TURN0;
			}
			wait_cnt = 150;
		}
		break;

	  case GUARD :									/* お互いガード */
		draw_screen();
		chr[0].motion[0][1] = 0;
		chr[1].motion[0][1] = 0;
		chr[0].attack = FALSE;
		chr[1].attack = FALSE;

		turn = 0;
		stat = TURN0;
		wait_cnt = 100;
		break;

	  case RESULT :									/* 結果 */
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
			play_music(seq_over);					/* 終了BGM再生 */
			move_cnt = 400;
		}
		else if ( (move_cnt > FADE_COUNT + 2) && (pcePadGet() & (TRG_A | TRG_B)) ) {
			move_cnt = FADE_COUNT + 2;
		}
		if ( --move_cnt == FADE_COUNT + 1 ) {		/* メニューを開く */
			menu_cnt = -2;
		}
		else if ( move_cnt == FADE_COUNT ) {
			fade_out();								/* 画面フェードアウト */
			fade_sound();							/* サウンドフェードアウト */
		}
		else if ( move_cnt < 0 ) {
			stat = START;
		}
		break;
	}
	init_flag = (stat != prev);						/* 状態遷移 */
}


/************
    初期化
 ************/
void	init_app(void)
{
	Sprite*	p;
	int		i;

	set_bmp(&bmp_sprite0, PAT_YOMYAMO0);			/* 透過色あり */
	set_bmp(&bmp_sprite1, PAT_YOMYAMO1);			/* 透過色無し */
	set_bmp(&bmp_pose, PAT_POSE);					/* キャラクタ */
	p = main_spr;
	for (i = 0; i < 4; i++) {										/* カード枠の隅 */
		set_sprite(p++, &bmp_sprite0, 16 + (i % 2)*4, 42 + i/2*4,  4,  4, DRW_NOMAL);
	}
	set_sprite(p++, &bmp_sprite1, 84,  0, 27, 27, DRW_NOMAL);		/* 裏面模様 */
	for (i = 0; i < ACT_MAX; i++) {									/* 行動 */
		set_sprite(p++, &bmp_sprite1, (i % 3)*28, i/3*19, 27, 19, DRW_NOMAL);
	}
	for (i = 0; i < 4; i++) {										/* 行動カウンタ */
		set_sprite(p++, &bmp_sprite1, 84 + i*8, 28, 8, 4, DRW_NOMAL);
	}
	set_sprite(p++, &bmp_sprite0,  24, 42, 16, 16, DRW_NOMAL);		/* ヒットマーク */
	set_sprite(p++, &bmp_sprite1, 112,  0,  8,  8, DRW_NOMAL);		/* 背景 */
	for (i = 0; i < 26 + 2; i++) {									/* フォント */
		set_sprite(p++, &bmp_sprite0, (i % 9)*14, i/9*14, 14, 14, DRW_NOMAL);
	}
	set_sprite(p++, &bmp_sprite1,  0, 57, 120, 40, DRW_NOMAL);		/* タイトル */

	for (i = 0; i < 5 + ACT_MAX; i++) {								/* 状態グラフィック */
		set_sprite(p++, &bmp_pose, 0, i*32, 32, 32, DRW_NOMAL);
	}

	for (i = 0; i < SE_MAX; i++) {					/* 効果音設定 */
		memcpy(se_info + i, se_table[i] + 8, sizeof(PCEWAVEINFO));
		se_info[i].pData = se_table[i] + 8 + sizeof(PCEWAVEINFO);
	}

	stat = TITLE;									/* 状態 */
	init_flag = TRUE;
	menu_cnt = 0;
}

/**********
    終了
 **********/
void	end_app(void)
{
	StopMusic();
	pceWaveStop(0);
}

/**************** End of File ********************************************/