
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>
#include <vector>

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>


#define CLANG_PATH "clang"
#define CLANGXX_PATH "clang++"
#define LLVM_AR_PATH "llvm-ar"


using namespace std;


typedef std::vector<std::string> path_list;
typedef std::map<std::string,std::string> md5_list;


int is_debug_output = 0;


const char* get_clang_path(void) {
    char* clang_path = getenv("KFL_CLANG");

    if (!clang_path)
        return CLANG_PATH;

    return clang_path;
}

const char* get_clangpp_path(void) {
    char* clangpp_path = getenv("KFL_CLANGXX");

    if (!clangpp_path)
        return CLANGXX_PATH;

    return clangpp_path;
}

const char* get_llvm_ar_path(void) {
    char* llvm_ar_path = getenv("KFL_LLVM_AR");

    if (!llvm_ar_path)
        return LLVM_AR_PATH;

    return llvm_ar_path;
}

const char* get_compiler_flags(void) {
    char* compiler_flags = getenv("KFL_CFLAG");

    if (!compiler_flags)
        return NULL;

    return compiler_flags;
}

std::string get_file_md5(const string& path) {
    std::string command = "md5 ";
    command += path;

    FILE* process_handle = popen(command.c_str(),"r");
    char md5_buffer[256] = {0};

    fread(&md5_buffer,1,sizeof(md5_buffer),process_handle);
    pclose(process_handle);

    std::string result = md5_buffer;
    unsigned int offset = result.find("=");

    if (-1 != offset) {
        result = result.substr(offset + 2);
        result = result.substr(0,result.length() - 1);

        return result;
    }

    return "";
}

path_list* list_dir(const string& path) {
    struct dirent* file_handle = NULL;
    DIR* dir_handle = opendir(path.c_str());

    if (NULL == dir_handle)
        return NULL;

    path_list* result = new path_list();

    while (NULL != (file_handle = readdir(dir_handle))) {
        std::string file_name = file_handle->d_name;

        if (4 == file_handle->d_type) {
            if ("." == file_name || ".." == file_name)
                continue;

            path_list* subdir_file = list_dir(path + "/" + std::string(file_handle->d_name));

            if (NULL == subdir_file)
                continue;

            for (path_list::iterator subdir_file_iterator = subdir_file->begin();subdir_file_iterator != subdir_file->end();++subdir_file_iterator)
                result->push_back(*subdir_file_iterator);

            delete subdir_file;
        } else {
            if (is_debug_output) {
                std::string current_path = path + "/" + file_name;

                printf("file_handle->d_name = %s file_handle->d_type = %d \n",current_path.c_str(),file_handle->d_type);
            }

            if (-1 != file_name.find(".bc") && -1 == file_name.find(".bca"))
                result->push_back(path + "/" + std::string(file_handle->d_name));
        }
    }

    return result;
}

path_list* filter_similar(path_list* file_list) {
    path_list* result = new path_list;
    md5_list md5_record_list;

    for (path_list::iterator file_list_iterator = file_list->begin();file_list_iterator != file_list->end();++file_list_iterator) {
        std::string md5_hash = get_file_md5(*file_list_iterator);

        if (is_debug_output)
            printf("file_path = %s ,file_md5 = %s\n",file_list_iterator->c_str(),md5_hash.c_str());

        if (!md5_record_list.count(md5_hash)) {

            md5_record_list[md5_hash] = *file_list_iterator;

            result->push_back(*file_list_iterator);
        } else {
            if (is_debug_output)
                printf("Exist Similar File  file_path = %s (%s) ,file_md5 = %s\n",file_list_iterator->c_str(),md5_record_list[md5_hash].c_str(),md5_hash.c_str());
        }

    }

    return result;
}

unsigned int print_dir_file(path_list* file_list) {
    unsigned int file_count = 0;

    printf("print_dir_file() Output:\n");

    for (path_list::iterator file_list_iterator = file_list->begin();file_list_iterator != file_list->end();++file_list_iterator) {
        printf("-> %s \n",file_list_iterator->c_str());

        file_count += 1;
    }

    return file_count;
}

void print_parameters(char** parameter_list) {
    printf("Argument : ");

    for (int index = 0;NULL != parameter_list[index];++index)
        printf("%s ",parameter_list[index]);

    printf("\n");
}

void print_help(void) {
    printf("Using : [ KFL_CFLAG=\"-I.\" ] klee-build -bc  %%Fuzzer_Path%% | -bf %%Project_Build_Dir%%\n");
    printf("  -bc : build fuzzer to LLVM BitCode\n");
    printf("  -bf : build fuzzer for klee fuzzing\n");
    printf("  Fuzzer_Path : Custom Fuzzer Path\n");
    printf("  Project_Build_Dir : Project Path\n");
    printf("  KFL_CFLAG : Custom Compiler Flags\n");
    printf("Example : \n");
    printf("  ./klee-build -bc ./test_code/test_fuzzing_entry.c\n");
    printf("  ./klee-build -bf ./test_code\n");
}

int compile_fuzzer_to_bitcode(const char* fuzzer_path) {
    unsigned call_parameters_length = sizeof(char*) * 32;
    char** call_parameters = (char**)malloc(call_parameters_length);
    const char* compiler_flags = get_compiler_flags();
    std::string output_path = fuzzer_path;

    if (-1 != output_path.rfind(".c"))
        output_path = output_path.substr(0,output_path.rfind(".c")) + ".bc";
    else if (-1 != output_path.rfind(".cpp"))
        output_path = output_path.substr(0,output_path.rfind(".cpp")) + ".bc";

    call_parameters[0] = (char*)get_clang_path();
    call_parameters[1] = (char*)"-g";
    call_parameters[2] = (char*)"-emit-llvm";
    call_parameters[3] = (char*)"-c";
    call_parameters[4] = (char*)fuzzer_path;
    call_parameters[5] = (char*)"-o";
    call_parameters[6] = (char*)output_path.c_str();

    unsigned int call_parameters_index = 0;

    if (NULL != compiler_flags) {
        std::string compiler_flags_string = compiler_flags;

        while (compiler_flags_string.length()) {
            char* temp_string = NULL;
            unsigned int space_offset = compiler_flags_string.find(" ");

            if (-1 != space_offset) {
                temp_string = (char*)malloc(space_offset + 1);

                memset(temp_string,0,space_offset + 1);
                memcpy(temp_string,compiler_flags_string.c_str(),space_offset);

                compiler_flags_string = compiler_flags_string.substr(0,space_offset + 1);
            } else {
                temp_string = (char*)malloc(compiler_flags_string.length() + 1);

                memset(temp_string,0,compiler_flags_string.length() + 1);
                memcpy(temp_string,compiler_flags_string.c_str(),compiler_flags_string.length());

                compiler_flags_string = "";
            }

            call_parameters[7 + call_parameters_index++] = temp_string;
        }
    }

    call_parameters[7 + call_parameters_index] = (char*)NULL;

    print_parameters(call_parameters);
    printf("execvp() = %d\n",execvp(call_parameters[0],call_parameters));
    printf("errno = %d\n",errno);

    free(call_parameters);

    if (errno)
        return 0;

    return 1;
}

int compile_fuzzer_to_lib(const char* project_path,path_list* llvm_bitcode_file_list,unsigned int file_count) {
    unsigned call_parameters_length = sizeof(char*) * 1024;
    char** call_parameters = (char**)malloc(call_parameters_length);
    std::string output_path = project_path;
    output_path += "/klee_fuzzer.bca";

    call_parameters[0] = (char*)get_llvm_ar_path();
    call_parameters[1] = (char*)"rsc";
    call_parameters[2] = (char*)output_path.c_str();

    unsigned int file_index = 0;
    path_list* filter_list = filter_similar(llvm_bitcode_file_list);

    for (path_list::iterator file_list_iterator = filter_list->begin();file_list_iterator != filter_list->end();++file_list_iterator,++file_index)
        call_parameters[3 + file_index] = (char*)file_list_iterator->c_str();

    call_parameters[3 + file_index] = NULL;

    print_parameters(call_parameters);

    if (!access(output_path.c_str(),F_OK))
        remove(output_path.c_str());


    printf("execvp() = %d\n",execvp(call_parameters[0],call_parameters));
    printf("errno = %d\n",errno);

    free(call_parameters);

    delete filter_list;

    if (errno)
        return 0;

    return 1;
}

int main(int argc,char** argv) {
    if (3 != argc) {
        print_help();

        return 1;
    }

    if (!strcmp(argv[1],"-bc")) {
        char* fuzzer_path = argv[2];

        printf("Ready to Compiler Fuzzer \n");

        if (!compile_fuzzer_to_bitcode(fuzzer_path)) {
            printf("Compile Fuzzer to BitCode Error ! ..\n");

            return 1;
        }
    } else if (!strcmp(argv[1],"-bf")) {
        char* project_path = argv[2];
        path_list* llvm_bitcode_file_list = list_dir(std::string(project_path));

        if (NULL == llvm_bitcode_file_list) {
            printf("project_path(%s) is Error Dir Path ..\n",project_path);

            return 1;
        }

        unsigned int file_count = print_dir_file(llvm_bitcode_file_list);

        printf("project .bc file count = %d \n",file_count);

        if (!file_count) {
            printf("project_path(%s) have not LLVM BitCode File ..\n",project_path);
            printf("Check it is you compile the project with klee-clang ?..");

            return 1;
        }

        printf("Ready to Compiler Lib \n");

        if (!compile_fuzzer_to_lib(project_path,llvm_bitcode_file_list,file_count)) {
            printf("Link Fuzzer to lib Error ! ..\n");

            return 1;
        }

        printf("Compile All Success ..\n");

        delete llvm_bitcode_file_list;
    }

    return 0;
}

