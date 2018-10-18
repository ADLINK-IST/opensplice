
namespace org { namespace opensplice { namespace topic {

typedef v_copyin_result (*copyInFunction)(c_type dbType, const void *, void *);
typedef void (*copyOutFunction)(const void *, void *);

template<>
class TopicTraits< ${proto-type-name} > {
public:
    static ::org::opensplice::topic::DataRepresentationId_t getDataRepresentationId()
    {
        return ::org::opensplice::topic::GPB_REPRESENTATION;
    }

    static ::std::vector<os_uchar> getMetaData()
    {
        ${protobuf-meta-descriptor}
    }

    static ::std::vector<os_uchar> getTypeHash()
    {
        ${protobuf-meta-hash}
    }

    static ::std::vector<os_uchar> getExtentions()
    {
        ${protobuf-extensions}
    }

    static const char *getKeyList()
    {
        return ::org::opensplice::topic::TopicTraits< ${dds-type-name} >::getKeyList();
    }

    static const char *getTypeName()
    {
        return ::org::opensplice::topic::TopicTraits< ${dds-type-name} >::getTypeName();
    }

    static std::string getDescriptor()
    {
        return ::org::opensplice::topic::TopicTraits< ${dds-type-name} >::getDescriptor();
    }

    static u_bool protobufToDds(const ${proto-type-name}* proto, ${dds-type-name} *dds)
    {
        bool success;
        ::std::string blob;

        if(proto->IsInitialized()){
            /* get protobuf fields that have been marked as key fields and set in dds */
            ${get-protobuf-fields}

            success = proto->SerializeToString(&blob);

            if(success){
                ::std::vector<os_uchar> bytes(blob.begin(), blob.end());
                dds->ospl_protobuf_data(bytes);
            }
        } else {
            success = false;
        }
        return (success==true);
    }

    static void ddsToProtobuf(${dds-type-name}* dds, ${proto-type-name}* proto)
    {
        bool success;

        if(!(dds->ospl_protobuf_data().empty())){
            ::std::vector<os_uchar> bytes = dds->ospl_protobuf_data();
            success = proto->ParseFromString(::std::string(bytes.begin(),bytes.end()));

            if(!success){
                ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to translate to GPB representation");
            }
        } else {
            /* get dds key fields and set in proto */
            ${set-protobuf-fields}
        }
        return;
    }

    static v_copyin_result protoCopyIn(c_type dbType, const void *from, void *to)
    {
        ${dds-type-name} dds;
        v_copyin_result result = V_COPYIN_RESULT_INVALID;

        copyInFunction copyIn = ::org::opensplice::topic::TopicTraits< ${dds-type-name} >::getCopyIn();

        if(protobufToDds((const ${proto-type-name}*)from, &dds)){
            result = copyIn(dbType, &dds , to);
        }
        return result;
    }

    static void protoCopyOut(const void *from, void *to)
    {
        ${dds-type-name} ddsTo;
        copyOutFunction copyOut;

        copyOut = ::org::opensplice::topic::TopicTraits< ${dds-type-name} >::getCopyOut();
        copyOut(from, &ddsTo);
        ddsToProtobuf(&ddsTo, (${proto-type-name}*)to);

        return;
    }

    static copyInFunction getCopyIn()
    {
        return protoCopyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return protoCopyOut;
    }
};

}
}
}

namespace dds { namespace topic {
template <>
struct topic_type_name< ${proto-type-name} >
{
    static ::std::string value()
    {
        return org::opensplice::topic::TopicTraits< ${proto-type-name} >::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(${proto-type-name})
