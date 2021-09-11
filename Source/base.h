
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<piece.h>
#include	<muslib.h>


typedef	int		Bool;

#define	TRUE	1
#define	FALSE	0

typedef	unsigned int	u_int;
typedef	unsigned short	u_short;
typedef	unsigned char	u_char;

/*** �X�v���C�g *******/
typedef struct {
	PIECE_BMP*	bmp;			/* �r�b�g�}�b�v�f�[�^ */
	int			sx;				/* �]�������W */
	int			sy;
	int			w;				/* �傫�� */
	int			h;
	int			param;			/* �`��p�����[�^ */
} Sprite;


extern u_char	vram[SCREEN_W*SCREEN_H];				/* ��ʃo�b�t�@ */
extern int		fade_flag;								/* ��ʃt�F�[�h��� */

#define		FADE_COUNT	32								/* �t�F�[�h�C���E�A�E�g�̎��� */
#define		fade_in()	{fade_flag = 1;}				/* �t�F�[�h�C�� */
#define		fade_out()	{fade_flag = -1;}				/* �t�F�[�h�A�E�g */

extern Bool		sound_flag;								/* �T�E���h�Đ��t���O */
extern int		snd_fade_cnt;							/* �T�E���h�t�F�[�h�J�E���^ */

extern int		common_counter;							/* �ėp�J�E���^ */
extern int		master_volume;							/* �S�̂̉��� */


#define	rnd(n)	(rand() % (n))							/* �����擾 */

extern void		draw_sprite(Sprite*, int, int);								/* �X�v���C�g�`�� */
extern void		set_sprite(Sprite*, PIECE_BMP*, int, int, int, int, int);	/* �X�v���C�g�ݒ� */
extern void		set_bmp(PIECE_BMP*, const u_char*);							/* BMP�ݒ� */

extern void		play_music(const u_char*);				/* BGM�Đ� */
extern void		fade_sound(void);						/* �T�E���h�t�F�[�h�A�E�g */

/****************** End of File *********************************************/