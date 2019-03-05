#include "CodeGen/ServiceInterfaceWrapperGenerator.h"
#include "CodeGen/UtilsGenerator.h"

#include <sstream>
#include <iostream>

namespace vazgen {

ServiceInterfaceWrapperGenerator::ServiceInterfaceWrapperGenerator(const ProtoFile& serviceProtoFile,
                                                                   const ProtoFile& dataProtoFile)
    : m_serviceProtoFile(serviceProtoFile)
    , m_serviceDataFile(dataProtoFile)
{
}

void ServiceInterfaceWrapperGenerator::generate()
{
    // TODO:
    for (const auto& [name, service] : m_serviceProtoFile.getServices()) {
        auto& wrapperHeader = m_generatedFiles[name + "header"];
        auto& wrapperSource = m_generatedFiles[name + "source"];
        wrapperHeader.setName(name + "Wrapper");
        wrapperHeader.setHeader(true);
        wrapperSource.setName(name + "Wrapper");
        wrapperSource.setHeader(false);
        wrapperSource.addInclude("\"" + wrapperHeader.getName() + "\"\n");
        for (const auto& rpc : service.getRPCs()) {
            const auto& wrapperF = generateWrapperFunction(rpc);
            wrapperSource.addFunction(wrapperF);
            wrapperHeader.addFunction(wrapperF);
        }
    }
    generateUtilFile();
}

Function ServiceInterfaceWrapperGenerator::generateWrapperFunction(const ProtoService::RPC& rpc)
{
    Function wrapperF(rpc.m_name);
    if (rpc.m_output.getFields().size() != rpc.m_input.getFields().size()) {
        // has return value
        const auto& returnField = rpc.m_output.getFields().back();
        Type returnType = {returnField.m_Ctype,
                           returnField.m_isConst ? "const" : "",
                           returnField.m_isPtr ? true : false,
                           false}; 
        wrapperF.setReturnType(returnType);
    }
    for (const auto& argField : rpc.m_input.getFields()) {
        Type argtype = {argField.m_Ctype,
                        argField.m_isConst ? "const" : "",
                        argField.m_isPtr ? true : false,
                        argField.m_isRepeated ? true : false};
        wrapperF.addParam(Variable{argtype, argField.m_name});
    }
    generateFunctionBody(wrapperF, rpc);
    return wrapperF;
}

void ServiceInterfaceWrapperGenerator::generateFunctionBody(Function& wrapperF,
                                                            const ProtoService::RPC& rpc)
{
    // generate input
    const std::string input = "input";
    const std::string output = "output";
    wrapperF.addBody(rpc.m_input.getName() + " " + input);
    for (const auto& field : rpc.m_input.getFields()) {
        std::stringstream utilCall;
        utilCall << "Utils::set_"
              << field.m_name << "(&"
              << field.m_name << ", "
              << "input);";
        wrapperF.addBody(utilCall.str());
    }
    wrapperF.addBody("grpc::ClientContext context;");
    wrapperF.addBody(rpc.m_output.getName() + " " + output);
    // call
    wrapperF.addBody("// Invoke service function");
    std::stringstream serviceCallStrm;
    serviceCallStrm << "stub_" << wrapperF.getName() << "("
                    << "&context," << input << ", " << "&" << output << ")";
    wrapperF.addBody(serviceCallStrm.str());

    // get output
    for (const auto& field : rpc.m_output.getFields()) {
        std::stringstream utilCall;
        utilCall << "Utils::get_"
              << field.m_name << "(&"
              << field.m_name << ", "
              << "output);";
        wrapperF.addBody(utilCall.str());
    }
    if (rpc.m_input.getFields().size() != rpc.m_output.getFields().size()) {
        wrapperF.addBody("return output." + rpc.m_output.getFields().back().m_name + "();");
    }
}

void ServiceInterfaceWrapperGenerator::generateUtilFile()
{
    UtilsGenerator utilsGenerator(m_serviceProtoFile.getName() + "Data", m_serviceProtoFile);
    utilsGenerator.setSettersFor(UtilsGenerator::INPUT);
    utilsGenerator.setGettersFor(UtilsGenerator::OUTPUT);
    utilsGenerator.addInclude("\"" + m_serviceDataFile.getPackage() + ".pb.h\"");
    utilsGenerator.generate();
    m_generatedFiles.insert(std::make_pair("utils_header", utilsGenerator.getHeader()));
    m_generatedFiles.insert(std::make_pair("utils_source", utilsGenerator.getSource()));
}

} // namespace vazgen

