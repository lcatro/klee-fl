

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include <string>


#define CONFIGURE_OUTPUT_TEST_FILE "conftest"
#define CONFIGURE_INPUT_TEST_FILE "conftest.c"
#define CLANG_PATH "clang"
#define CLANGXX_PATH "clang++"
#define LLVM_AR_PATH "llvm-ar"
#define LLVM_COMPILER_PATH "/usr/bin/"


using namespace std;


const char* get_llvm_compiler_path(void) {
    char* llvm_compiler_path = getenv("LLVM_COMPILER_PATH");

    if (!llvm_compiler_path)
        return LLVM_COMPILER_PATH;

    return llvm_compiler_path;
}

std::string get_clang_path(void) {
    const char* clang_path = getenv("KFL_CLANG");

    if (!clang_path)
        clang_path = CLANG_PATH;

    const char* llvm_path = get_llvm_compiler_path();
    std::string result_path = llvm_path;

    result_path += "/";
    result_path += clang_path;

    if (!access(result_path.c_str(),F_OK))
        return result_path;

    result_path = clang_path;

    return result_path;
}

std::string get_clangpp_path(void) {
    const char* clangpp_path = getenv("KFL_CLANGXX");

    if (!clangpp_path)
        clangpp_path = CLANGXX_PATH;

    const char* llvm_path = get_llvm_compiler_path();
    std::string result_path = llvm_path;

    result_path += "/";
    result_path += clangpp_path;

    if (!access(result_path.c_str(),F_OK))
        return result_path;

    result_path = clangpp_path;

    return result_path;
}

std::string get_llvm_ar_path(void) {
    const char* llvm_ar_path = getenv("KFL_LLVM_AR");

    if (!llvm_ar_path)
        llvm_ar_path = LLVM_AR_PATH;

    const char* llvm_path = get_llvm_compiler_path();
    std::string result_path = llvm_path;

    result_path += "/";
    result_path += llvm_ar_path;

    if (!access(result_path.c_str(),F_OK))
        return result_path;

    result_path = llvm_ar_path;

    return result_path;
}

std::string get_compiler_flags(void) {
    const char* compiler_flags = getenv("KFL_CFLAG");
    std::string result;

    if (!compiler_flags)
        return result;

    result = compiler_flags;

    return result;
}

void print_parameters(char** parameter_list) {
    printf("Argument : ");

    for (int index = 0;NULL != parameter_list[index];++index)
        printf("%s ",parameter_list[index]);

    printf("\n");
}

char* execute(const char* command) {
    FILE* process_handle = popen(command,"r");
    unsigned int buffer_length = 0;

    fseek(process_handle,0,SEEK_END);

    buffer_length = ftell(process_handle);

    fseek(process_handle,0,SEEK_SET);

    char* md5_buffer = (char*)malloc(buffer_length + 1);

    memset(md5_buffer,0,buffer_length + 1);
    fread(md5_buffer,1,buffer_length,process_handle);
    pclose(process_handle);

    return md5_buffer;
}

void replace_to_llvm_bitcode_output(const char* source_name,char* output_path) {
    if (strstr(source_name,".o")) {  //  build LLVM Bitcode ..
        printf("build LLVM Bitcode  .o \n");
        memcpy(output_path,source_name,strlen(source_name) - 2);
        strcat(output_path,".bc");
    } else if (strstr(source_name,".a")) {  //  build LLVM Bitcode library ..
        printf("build LLVM Bitcode library  .a \n");
        memcpy(output_path,source_name,strlen(source_name) - 2);
        strcat(output_path,".bca");
    } else if (strstr(source_name,".c") || strstr(source_name,".cpp")) {  //  build LLVM Bitcode ..
        printf("build LLVM Bitcode library  .c / .cpp \n");
        memcpy(output_path,source_name,strlen(source_name) - 2);
        strcat(output_path,".bc");
    } else if (!strstr(source_name,".")) {
        printf("build LLVM Bitcode library  binary \n");
        memcpy(output_path,source_name,strlen(source_name));
        strcat(output_path,".bca");
    } else {
        memcpy(output_path,source_name,strlen(source_name));
    }
}

std::string build_klee_bitcode_compile_parameter(int argc,char** argv) {
    unsigned call_parameters_length = argc * sizeof(char*) * 64;
    char** call_parameters = (char**)malloc(call_parameters_length);
    unsigned link_input_file_length = argc * sizeof(char*) * 64;
    char** link_input_file_list = (char**)malloc(link_input_file_length);
    char* output_file_path = NULL;
    char* input_file_path = NULL;

    memset(call_parameters,0,call_parameters_length);
    memset(link_input_file_list,0,link_input_file_length);

    int argv_index = 1;
    int input_file_count = 0;
    int is_bingo_flag_version = 1;
    int is_bingo_flag_emit_llvm = 0;
    int is_bingo_flag_bitcode = 0;
    int is_bingo_flag_debug_output = 0;
    int is_llvm_ar_link = 0;
    int is_compile_object = 0;
    int have_special_output_path = 0;
    int is_mutil_input_file = 0;

    for (;argv_index < argc;++argv_index) {
        if (!strcmp("-v",argv[argv_index]) || !strcmp("--version",argv[argv_index])) {
            is_bingo_flag_version = 1;
        } else if (!strcmp("-emit-llvm",argv[argv_index])) {
            is_bingo_flag_emit_llvm = 1;
        } else if (!strcmp("-c",argv[argv_index])) {
            is_bingo_flag_bitcode = 1;
        } else if (!strcmp("-g",argv[argv_index])) {
            is_bingo_flag_debug_output = 1;
        } else if (!strcmp("-o",argv[argv_index])) {
            if (argv_index + 1 >= argc) {
                printf("Error ! Parameter -o lost target ..\n");

                exit(1);
            }

            call_parameters[argv_index] = argv[argv_index];
            argv_index += 1;

            printf("Check Output File : %s\n",argv[argv_index]);

            if (!strcmp(argv[argv_index],CONFIGURE_OUTPUT_TEST_FILE)) {  //  ./configure comand : ../klee-clang -o conftest conftest.c  >&5
                printf("Check : is CONFIGURE_TEST_FILE in -o ,ignore ..\n");

                call_parameters[argv_index] = argv[argv_index];
                output_file_path = argv[argv_index];
            } else {
                char* output_path = (char*)malloc(64);

                memset(output_path,0,64);
                replace_to_llvm_bitcode_output(argv[argv_index],output_path);

                if (strstr(output_path,".bca"))
                    is_llvm_ar_link = 1;

                call_parameters[argv_index] = output_path;
                output_file_path = output_path;
            }

            have_special_output_path = 1;

            continue;
        } else if (strstr(argv[argv_index],".o") || strstr(argv[argv_index],".a")) {  //  Link Code files .o and .a
            char* output_path = (char*)malloc(64);

            memset(output_path,0,64);
            replace_to_llvm_bitcode_output(argv[argv_index],output_path);

            call_parameters[argv_index] = output_path;
            link_input_file_list[input_file_count++] = output_path;

            is_compile_object = 1;

            continue;
        } else if (strstr(argv[argv_index],".c") || strstr(argv[argv_index],".cpp")) {  //  Compile File ..
            if (!strcmp(argv[argv_index],CONFIGURE_INPUT_TEST_FILE)) {  //  ./configure comand : ../klee-clang conftest.c >&5
                have_special_output_path = 1;
                input_file_path = (char*)CONFIGURE_INPUT_TEST_FILE;
            } else {
                is_compile_object = 1;

                if (NULL == input_file_path) {
                    unsigned int input_file_path_length = strlen(argv[argv_index]);
                    input_file_path = (char*)malloc(input_file_path_length + 1);

                    memcpy(input_file_path,argv[argv_index],input_file_path_length);

                    input_file_path[input_file_path_length] = 0;
                } else {
                    is_mutil_input_file = 1;
                }
            }
        }

        call_parameters[argv_index] = argv[argv_index];
    }
    
    if (is_llvm_ar_link) {
        std::string llvm_ar_path = get_llvm_ar_path();

        call_parameters[0] = (char*)llvm_ar_path.c_str();
        call_parameters[1] = (char*)"rcs";
        call_parameters[2] = output_file_path;

        int input_file_index = 0;

        for (;input_file_index < input_file_count;++input_file_index)
            call_parameters[input_file_index + 3] = link_input_file_list[input_file_index];

        call_parameters[input_file_index + 3] = NULL;
    } else {
        std::string clang_path = get_clang_path();

        call_parameters[0] = (char*)clang_path.c_str();

        if (is_compile_object) {
            if (!is_bingo_flag_version) {
                call_parameters[argv_index++] = (char*)"-v";
            }
            if (!is_bingo_flag_emit_llvm) {
                call_parameters[argv_index++] = (char*)"-emit-llvm";
            }
            if (!is_bingo_flag_bitcode) {
                call_parameters[argv_index++] = (char*)"-c";
            }
            if (!is_bingo_flag_debug_output) {
                call_parameters[argv_index++] = (char*)"-g";
            }
            if (!have_special_output_path && NULL != input_file_path && !is_mutil_input_file) {
                char* output_path = (char*)malloc(256);

                memset(output_path,0,256);
                replace_to_llvm_bitcode_output(input_file_path,output_path);

                call_parameters[argv_index++] = (char*)"-o";
                call_parameters[argv_index++] = output_path;
            }
        }

        call_parameters[argv_index] = NULL;
    }
    
    printf("is_compile_object = %d,is_bingo_flag_emit_llvm = %d ,is_bingo_flag_bitcode = %d ,is_bingo_flag_debug_output = %d ,have_special_output_path = %d ,is_mutil_input_file=%d\n",
        is_compile_object,is_bingo_flag_emit_llvm,is_bingo_flag_bitcode,is_bingo_flag_debug_output,have_special_output_path,is_mutil_input_file);
    //print_parameters(call_parameters);
    
    std::string result;

    for (int index = 0;NULL != call_parameters[index];++index) {
        result += call_parameters[index];
        result += " ";
    }

    if (!is_llvm_ar_link) {
        std::string compiler_flags = get_compiler_flags();

        if (!compiler_flags.empty())
            result += compiler_flags;
    }

    printf("Argument : %s\n",result.c_str());

    return result;
}

std::string build_compiler_parameter(int argc,char** argv) {
    std::string result = get_clang_path();

    result += " ";

    for (int index = 1;index < argc;++index) {
        result += argv[index];
        result += " ";
    }

    return result;
}


int main(int argc,char** argv) {
    printf(">>>>> Entry Klee-Clang >>>>>\n");
    print_parameters(argv);

    std::string source_parameters = build_compiler_parameter(argc,argv);
    std::string klee_compile_parameters = build_klee_bitcode_compile_parameter(argc,argv);

    printf("Source Compile Command:\n");
    
    execute(source_parameters.c_str());

    printf("errno = %d\n",errno);
    printf("Klee Compile Command:\n");

    execute(klee_compile_parameters.c_str());

    printf("errno = %d\n",errno);
    printf("<<<<< Exit Klee-Clang <<<<<\n");

    return 0;
}

