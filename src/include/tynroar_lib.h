#include <stdbool.h>

#ifndef TYNROARLIB_H
#define TYNROARLIB_H

typedef enum {
  STAGEFLAG_DEFAULT = (1u << 0),
  STAGEFLAG_DISABLEDDRAW = (1u << 1),
  STAGEFLAG_DISABLEDSTEP = (1u << 2),
  STAGEFLAG_DISABLED = (STAGEFLAG_DISABLEDDRAW | STAGEFLAG_DISABLEDSTEP),
  STAGEFLAG_BLOCKSTEP = (1u << 3),
  STAGEFLAG_BLOCKDRAW = (1u << 4),
} STAGEFLAG;

typedef enum {
	CMDFLAG_DEFAULT = (1u << 0),
	CMDFLAG_BROADCAST = (1u << 1),
} CMDFLAF;

#define TYNFRAME \
	void (*dispose)(void *); \
	STAGEFLAG (*step)(void *, STAGEFLAG); \
	void (*draw)(void *); \
	char *(*cmdin)(void *, CMDFLAF *);                                                      \
	char *(*cmdout)(void *); \

typedef struct {
  TYNFRAME
  //fStep AppStep;
  //fDraw draw;
} TynFrame;

typedef struct TynStage {
  STAGEFLAG flags;
  TynFrame frame;
  void *state;
} TynStage;


#endif