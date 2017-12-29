// Split operation...

#ifndef csmsplit_hxx
#define csmsplit_hxx

enum csmsplit_opresult_t
{
    CSMSPLIT_OPRESULT_OK,
    CSMSPLIT_OPRESULT_NO,
    CSMSPLIT_OPRESULT_IMPROPER_CUT  // Due to tolerance error, pendant null edges to be joined
};

typedef enum csmsplit_opresult_t csmsplit_result;

#endif /* csmsplit_hxx */
