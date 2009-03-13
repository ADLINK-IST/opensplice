
#include "d_nameSpacesRequest.h"
#include "d_message.h"
#include "os_heap.h"

d_nameSpacesRequest
d_nameSpacesRequestNew(
    d_admin admin)
{
    d_nameSpacesRequest request;
    
    request = NULL;
    
    if(admin){
        request = d_nameSpacesRequest(os_malloc(C_SIZEOF(d_nameSpacesRequest)));
        
        if(request){
            d_messageInit(d_message(request), admin);
        }
    }
    return request;
}

void
d_nameSpacesRequestFree(
    d_nameSpacesRequest request)
{
    if(request){
        d_messageDeinit(d_message(request));
        os_free(request);
    }
}
