#ifndef OMO_TEST_H
#define OMO_TEST_H

#define OMO_TEST_STATE_LIBRARY_LOAD       0
#define OMO_TEST_STATE_LIBRARY            1
#define OMO_TEST_STATE_QUEUE              2
#define OMO_TEST_STATE_PLAYER             3
#define OMO_TEST_STATE_LIBRARY_AND_PLAYER 4
#define OMO_TEST_STATE_QUEUE_AND_PLAYER   5
#define OMO_TEST_STATE_ALL                6
#define OMO_TEST_STATE_EXIT               7

#define OMO_TEST_INTERVAL                60

bool omo_test_init(void * data, int mode, const char * path);
bool omo_test_logic(void * data);
void omo_test_exit(void * data);

#endif
