#ifndef FSMACOSX_ALERT_IS_INCLUDED
#define FSMACOSX_ALERT_IS_INCLUDED
/* { */

#ifdef __cplusplus
extern "C" {
#endif

void FsMessageDialogC(const char title[],const char msg[],const char *okBtnTxt);
int FsYesNoDialogC(const char title[],const char msg[],const char *okBtnTxt,const char *cancelBtnTxt);

#ifdef __cplusplus
}
#endif

/* } */
#endif
