// #include <common.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <elf.h>
// #include <isa.h>
// #include <memory/paddr.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <libelf.h>
// #include <gelf.h>
// typedef struct {
//     char name[32]; // save func name
//     paddr_t addr;
//     unsigned char info;
//     Elf32_Word size;
// } SymTable;

// SymTable *sym_tbl = NULL; 
// static int func_num = 0;
// static int rec_depth = 1;

#include <memory/host.h>
#include <memory/vaddr.h>
#include <device/map.h>
#include <common.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
#include <stdio.h>
#include <stdlib.h>
//#include <libelf.h>
//#include <gelf.h>


typedef struct {
    char name[32]; // 函数名
    uint32_t addr;  // 函数地址
    unsigned char info; // 符号信息
    Elf32_Word size;  // 函数大小
} SymTable;

// #define MAX_FUNCS 1024  

SymTable *sym_tbl;
int func_num = 0;
int func_depth=1;


void parse_elf(const char *elf_file) {
    if (elf_file == NULL) {
        printf("ELF file name is NULL\n");
        return;
    }

    FILE *fp = fopen(elf_file, "rb");
    if (fp == NULL) {
        printf("Failed to open ELF file\n");
        exit(0);
    }

    Elf32_Ehdr ehdr;
    if (fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp) <= 0) {
        printf("Failed to read the ELF header\n");
        fclose(fp);
        exit(0);
    }

    if (ehdr.e_ident[0] != 0x7f || ehdr.e_ident[1] != 'E' || 
        ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
        printf("The opened file isn't a ELF file\n");
        fclose(fp);
        exit(0);
    }

    fseek(fp, ehdr.e_shoff, SEEK_SET);
    Elf32_Shdr shdr;
    char *str_tab = NULL;

    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (fread(&shdr, sizeof(Elf32_Shdr), 1, fp) <= 0) {
            printf("Failed to find the shdr\n");
            fclose(fp);
            free(str_tab);
            exit(0);
        }

        if (shdr.sh_type == SHT_STRTAB) {
            str_tab = malloc(shdr.sh_size);
            if (!str_tab) {
                fprintf(stderr, "Failed to allocate memory for string table\n");
                fclose(fp);
                exit(0);
            }
            fseek(fp, shdr.sh_offset, SEEK_SET);
            if (fread(str_tab, shdr.sh_size, 1, fp) <= 0) {
                printf("Failed to find the string_table\n");
                fclose(fp);
                free(str_tab);
                exit(0);
            }
        }
    }

    fseek(fp, ehdr.e_shoff, SEEK_SET);
    Elf32_Sym sym;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (fread(&shdr, sizeof(Elf32_Shdr), 1, fp) <= 0) {
            printf("Failed to find the shdr\n");
            fclose(fp);
            free(str_tab);
            free(sym_tbl);
            exit(0);
        }

        if (shdr.sh_type == SHT_SYMTAB) {
            size_t symcount = shdr.sh_size / shdr.sh_entsize;
            sym_tbl = realloc(sym_tbl, symcount * sizeof(SymTable));
            if (!sym_tbl) {
                fprintf(stderr, "Failed to allocate memory for symbol table\n");
                fclose(fp);
                free(str_tab);
                exit(0);
            }
            fseek(fp, shdr.sh_offset, SEEK_SET);

            for (size_t j = 0; j < symcount; j++) {
                if (fread(&sym, sizeof(Elf32_Sym), 1, fp) <= 0) {
                    printf("Failed to find the symbol table\n");
                    fclose(fp);
                    free(str_tab);
                    free(sym_tbl);
                    exit(0);
                }
                if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC) {
                    const char* name = str_tab + sym.st_name;
                    if (sym.st_name >= shdr.sh_size) {
                        fprintf(stderr, "String table index out of range\n");
                        fclose(fp);
                        free(str_tab);
                        free(sym_tbl);
                        exit(0);
                    }
                    strncpy(sym_tbl[func_num].name, name, sizeof(sym_tbl[func_num].name) - 1);
                    sym_tbl[func_num].name[sizeof(sym_tbl[func_num].name) - 1] = '\0'; 
                    sym_tbl[func_num].addr = sym.st_value;
                    sym_tbl[func_num].size = sym.st_size;
                    func_num++;
                }
            }
        }
    }

    fclose(fp);
    free(str_tab);
}










    // // 寻找字符串表
    // Elf_Scn *str_scn = NULL;
    // for (Elf_Scn *scn = NULL; (scn = elf_nextscn(elf, scn)) != NULL;) 
    // {
    //     GElf_Shdr shdr;
    //     if (gelf_getshdr(scn, &shdr) != &shdr) {
    //         fprintf(stderr, "Failed to get section header: %s\n", elf_errmsg(-1));
    //         continue;
    //     }
    //     if (shdr.sh_type == SHT_STRTAB && shdr.sh_name != 0) {
    //         str_scn = scn;
    //         break;
    //     }
    // }

    // if (str_scn == NULL) {
    //     fprintf(stderr, "String table not found\n");
    //     elf_end(elf);
    //     close(fd);
    //     return;
    // }

    // Elf_Data *str_data = elf_getdata(str_scn, NULL);
    // if (str_data == NULL) {
    //     fprintf(stderr, "Failed to get string table data: %s\n", elf_errmsg(-1));
    //     elf_end(elf);
    //     close(fd);
    //     return;
    // }
    // char *strtab = (char *)str_data->d_buf;

    // // 寻找符号表
    // for (Elf_Scn *scn = NULL; (scn = elf_nextscn(elf, scn)) != NULL;) {
    //     GElf_Shdr shdr;
    //     if (gelf_getshdr(scn, &shdr) != &shdr) {
    //         fprintf(stderr, "Failed to get section header: %s\n", elf_errmsg(-1));
    //         continue;
    //     }
    //     if (shdr.sh_type == SHT_SYMTAB) {
    //         Elf_Data *data = elf_getdata(scn, NULL);
    //         if (data == NULL) {
    //             fprintf(stderr, "Failed to get section data: %s\n", elf_errmsg(-1));
    //             continue;
    //         }

    //         size_t sym_count = shdr.sh_size / sizeof(Elf32_Sym);
    //         sym_tbl = malloc(sym_count * sizeof(SymTable));
    //         if (sym_tbl == NULL) {
    //             perror("Failed to allocate memory for sym_tbl");
    //             elf_end(elf);
    //             close(fd);
    //             return;
    //         }

    //         for (size_t i = 0; i < sym_count; i++) {
    //             GElf_Sym sym;
    //             gelf_getsym(data, i, &sym);
    //             if (GELF_ST_TYPE(sym.st_info) == STT_FUNC) {
    //                 if (func_num < MAX_FUNCS) {
    //                     const char *name = strtab + sym.st_name;
    //                     strncpy(sym_tbl[func_num].name, name, sizeof(sym_tbl[func_num].name) - 1);
    //                     sym_tbl[func_num].addr = sym.st_value;
    //                     sym_tbl[func_num].info = sym.st_info;
    //                     sym_tbl[func_num].size = sym.st_size;
    //                     func_num++;
    //                 }
    //             }
    //         }
    //         break; 
    //     }
    // }

    // elf_end(elf);
    // close(fd);




void call_display(word_t pc, word_t func_addr) 
{
    word_t i=0;
    for(; i<func_num;i++)
    {
      if(func_addr >= sym_tbl[i].addr && func_addr < (sym_tbl[i].addr + sym_tbl[i].size))
        {
            break;
        }
    }
    printf("0X%08x:", pc);
    for(int k = 0; k < func_depth*2; k++) printf("  ");

    func_depth++;

    printf("call [%s@0X%08x]\n", sym_tbl[i].name, func_addr);

}

void ret_display(word_t pc, word_t func_addr) 
{
int i = 0;
    for(; i < func_num; i++)
    {
        if(func_addr >= sym_tbl[i].addr && func_addr < (sym_tbl[i].addr + sym_tbl[i].size))
        {
            break;
        }
    }
    printf("0X%08x:", pc);

    func_depth--;

    for(int k = 0; k <  func_depth*2; k++) printf("  ");

     printf("ret  [%s@0X%08x]\n", sym_tbl[i].name, func_addr);


}



void dtrace_read_display(IOMap *map, paddr_t addr ,int len)
{
  printf("address (" FMT_PADDR ") the device is {%s}  at pc = " FMT_WORD "the len =%i\n", addr, map->name,len, cpu.pc);

}

void dtrace_write_display(IOMap *map, paddr_t addr,int len,word_t data)
{
  printf("address (" FMT_PADDR ") the device is {%s}  the data is {0X%08x} at pc = " FMT_WORD "the len =%i\n", addr, map->name,data,len, cpu.pc);

}












