#
#	「よむやも」
#


.SUFFIXES:  .o .s .c


# 生成コマンド・オプションのマクロ
CC = pcc33
CFLAGS = -c -g -gp=0x0 -near -O2 -Wall
AS = pcc33
ASFLAGS = -c -g -gp=0x0 -near
LD = pcc33
LDFLAGS = -g -ls -lm

# 生成規則
.c.o:
	$(CC) $(CFLAGS) $<
.s.o:
	$(AS) $(ASFLAGS) $<

# 構成ファイル・生成ファイルのマクロ
PRGNAME  = yomyamo
FILENAME = yomyamo
CAPTION  = よむやも
OBJS     = yomyamo.o base.o pattern.o sound.o wavetbl.o

$(PRGNAME).srf : $(OBJS)
	$(LD) $(LDFLAGS) -e$(PRGNAME).srf $(OBJS)

# フラッシュ書き込みイメージ生成
pex : $(PRGNAME).srf
	ppack -e $(PRGNAME).srf -o$(FILENAME).pex -n$(CAPTION) -isprite\icon.pid

# 依存関係
yomyamo.o	: yomyamo.c base.h pattern.h sound.h
base.o		: base.c base.h
pattern.o	: pattern.c pattern.h sprite\pat_yomyamo0.c
sound.o		: sound.c sound.h seq\battle1.c wave\adpcm_attack.c
wavetbl.o	: wavetbl.c


# クリーンアップ
clean:
	del $(PRGNAME).srf $(FILENAME).pex $(PRGNAME).sym $(PRGNAME).map *.o

