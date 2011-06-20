#ifdef __cplusplus
extern "C" {
#endif


void InitLPT(void);
int  CheckLPT(void);
void CleanupLPT(void);
void LPTOut(int val, int port);



#ifdef __cplusplus
}
#endif
