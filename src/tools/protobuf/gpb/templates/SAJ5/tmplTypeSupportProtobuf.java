package ${package-name};

public class ${typesupport-claz-name}
        extends
        org.opensplice.dds.type.TypeSupportProtobuf<${proto-type-name}, ${dds-type-name}> {
    @Override
    public ${proto-type-name} ddsToProtobuf(
            ${dds-type-name} ddsData) {
        try {
            return ${proto-type-name}.parseFrom(ddsData.ospl_protobuf_data);
        } catch (com.google.protobuf.InvalidProtocolBufferException e) {
            throw new org.opensplice.dds.core.IllegalArgumentExceptionImpl(
                    this.environment, e.getMessage());
        }
    }

    @Override
    public ${dds-type-name} protobufToDds(
            ${proto-type-name} protobufData) {
        if(protobufData == null){
            return null;
        }
        return new ${dds-type-name}(
                ${get-protobuf-fields}
                protobufData.toByteArray());
    }

    @Override
    public ${proto-type-name} ddsKeyToProtobuf(
            ${dds-type-name} ddsData) {
        ${set-protobuf-key-fields}
    }

    public ${typesupport-claz-name}(
            org.opensplice.dds.core.OsplServiceEnvironment environment, String typeName) {
        super(environment,
                ${proto-type-name}.class,
                new org.opensplice.dds.type.TypeSupportImpl<${dds-type-name}>(
                        environment,
                        ${dds-type-name}.class,
                        typeName),
                initMetaDescriptor(),
                initMetaHash());
    }

    private static byte[] initMetaDescriptor(){
        return ${protobuf-meta-descriptor};
    }

    private static byte[] initMetaHash(){
        return ${protobuf-meta-hash};
    }
}
