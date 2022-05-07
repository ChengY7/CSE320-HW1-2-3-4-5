/*
 * PBX: simulates a Private Branch Exchange.
 */
#include <stdlib.h>
#include "pbx.h"
#include "debug.h"
#include "csapp.h"

/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
sem_t mutex;
sem_t shutdown_mutex;
typedef struct pbx {
    TU *tuArray[PBX_MAX_EXTENSIONS];
    int count;
} PBX;

#if 1
PBX *pbx_init() {
    PBX *pbx=malloc(sizeof(PBX));
    pbx->count=0;
    sem_init(&mutex, 0, 1);
    sem_init(&shutdown_mutex, 0, 1);
    return pbx;
}
#endif

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */
#if 1
void pbx_shutdown(PBX *pbx) {
    P(&mutex);
    for(int i=0; i<PBX_MAX_EXTENSIONS; i++) {
        if(pbx->tuArray[i]!=NULL) {
            shutdown(tu_fileno(pbx->tuArray[i]), SHUT_RD);
        }
    }
    V(&mutex);
    P(&shutdown_mutex);
    free(pbx);
    V(&shutdown_mutex);
    sem_destroy(&shutdown_mutex);
    sem_destroy(&mutex);
}
#endif

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
#if 1
int pbx_register(PBX *pbx, TU *tu, int ext) {
    P(&mutex);
    if(pbx->tuArray[ext]!=NULL) {
        V(&mutex);
        return -1;
    }
    tu_set_extension(tu, ext);
    pbx->tuArray[ext]=tu;
    tu_ref(tu, NULL);
    dprintf(ext, "%s %d\n", tu_state_names[TU_ON_HOOK], ext);
    pbx->count++;
    if(pbx->count==1) 
        P(&shutdown_mutex);
    V(&mutex);
    return 0;
}
#endif

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */
#if 1
int pbx_unregister(PBX *pbx, TU *tu) {
    P(&mutex);
    tu_hangup(tu);
    pbx->tuArray[tu_extension(tu)]=NULL;
    tu_unref(tu, NULL);
    pbx->count--;
    if(pbx->count==0) {
        V(&shutdown_mutex);
    }
    V(&mutex);
    return 0;
}
#endif

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */
#if 1
int pbx_dial(PBX *pbx, TU *tu, int ext) {
    P(&mutex);
    tu_dial(tu, pbx->tuArray[ext]);
    V(&mutex);
    return 0;
}
#endif
