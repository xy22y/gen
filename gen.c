
/* password generator by xy22y */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <sys/random.h>

#define min(_a,_b) ((_a)>(_b)) ? (_b) : (_a)
#define max(_a,_b) ((_a)>(_b)) ? (_a) : (_b)
#define ASZ(_x) (sizeof(_x)/sizeof(_x[0]))

const char* lower_case_chars="abcdefghijklmnopqrstuvwxyz";
const char* upper_case_chars="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* number_chars="0123456789";
const char* hex_chars[]={"",
                         "01234567890abcdef",
                         "01234567890ABCDEF",
                         "01234567890ABCDEFabcdef"};
const char* special_chars="+-=_@#$%^&;:,.<>()[]/~\\";
const char* hard_to_read_chars="0Oo1lI|B85Ss2Z$!()[]/\\,.;:-~_";

typedef struct _CONFIG {
    int password_length;
    int verbose;
    int exclude_hard_to_reads;
    int hex_only; // 0=no 1=lower 2=upper 3=both

    int min_lower_case_chars;
    int max_lower_case_chars;
    int min_upper_case_chars;
    int max_upper_case_chars;
    int min_number_chars;
    int max_number_chars;
    int min_special_chars;
    int max_special_chars;

    char* custom_chars;
    char* custom_exclude_chars;
    char* first_must_be_chars;
} CONFIG;

void get_rc_options(CONFIG* config);
void usage(CONFIG* config);
void class_count_argv(CONFIG* config,char* argv,int* min,int* max);
void print_char_count(char* desc,int min,int max);
char* plimit(char* b,size_t s,int min,int max);
void strrmchars(char* st,char* deletes);
void strrmdups(char* st);

//
// entry
//

int
main(int argc,char** argv) {
    unsigned int tmp;
    unsigned int ceil;
    int l;
    int c;
    int m;
    char* p;
    int count_lower_case_chars=0;
    int count_upper_case_chars=0;
    int count_number_chars=0;
    int count_special_chars=0;
    char char_pool[257];
    char password[257];

    // default options
    CONFIG config={
        20, // paswword_length
        0,  // verbose
        0,  // exclude_hard_to_reads
        0,  // hex_only

        -1, // min_lower_case_chars
        -1, // max_lower_case_chars
        -1, // min_upper_case_chars
        -1, // max_upper_case_chars
        -1, // min_number_chars
        -1, // max_number_chars
        -1, // min_special_chars
        -1, // max_special_chars

        NULL, // custom_chars
        NULL, // custom_exclude_chars
        NULL, // first_must_be_chars
    };

    get_rc_options(&config);

    for(argc--,argv++; argc; argc--,argv++) {
        if((*argv)[0]!='-') usage(&config);
        switch((*argv)[1]) {
        case 'c':
            config.exclude_hard_to_reads=1;
            break;
        case 'v':
            config.verbose=!config.verbose; // in case .genrc has it turned on
            break;
        case 'u':
            ++argv;--argc;
            class_count_argv(&config,*argv,
                             &config.min_upper_case_chars,
                             &config.max_upper_case_chars);
            break;
        case 'l':
            ++argv;--argc;
            class_count_argv(&config,*argv,
                             &config.min_lower_case_chars
                             ,&config.max_lower_case_chars);
            break;
        case 'n':
            ++argv;--argc;
            class_count_argv(&config,*argv,
                             &config.min_number_chars,
                             &config.max_number_chars);
            break;
        case 's':
            ++argv;--argc;
            class_count_argv(&config,*argv,
                             &config.min_special_chars,
                             &config.max_special_chars);
            break;
        case 'L':
            ++argv;--argc;
            config.password_length=atoi(*argv);
            break;
        case 'x':
            ++argv;--argc;
            if(argc==0) {
                config.hex_only=1;
                goto continue1;
            }
            if((*argv)[0]=='-') {
                config.hex_only=1;
                --argv;++argc;
                continue;
            }
            config.hex_only=atoi(*argv);
            break;
        case 'p':
            ++argv;--argc;
            if(*argv==NULL) usage(&config);
            config.custom_chars=strdup(*argv);
            break;
        case 'e':
            ++argv;--argc;
            if(*argv==NULL) usage(&config);
            config.custom_exclude_chars=strdup(*argv);
            break;
        case 'f':
            ++argv;--argc;
            if(*argv==NULL) usage(&config);
            config.first_must_be_chars=strdup(*argv);
            break;
        default:
            usage(&config);
        }
    }
continue1:
    memset(char_pool,0,sizeof(char_pool));
    if(config.custom_chars) {
        strcat(char_pool,config.custom_chars);
    } else
    if(config.hex_only) {
        if(config.hex_only<1||config.hex_only>=ASZ(hex_chars)) {
            usage(&config);
        }
        strcat(char_pool,hex_chars[config.hex_only]);
    } else {
        if(config.max_upper_case_chars) strcat(char_pool,upper_case_chars);
        if(config.max_lower_case_chars) strcat(char_pool,lower_case_chars);
        if(config.max_number_chars) strcat(char_pool,number_chars);
        if(config.max_special_chars) strcat(char_pool,special_chars);
        if(config.exclude_hard_to_reads)
            strrmchars(char_pool,(char*)hard_to_read_chars);
    }
    if(config.custom_exclude_chars)
        strrmchars(char_pool,config.custom_exclude_chars);
    strrmdups(char_pool);
    l=strlen(char_pool);
    if(!l) usage(&config);
    ceil=UINT_MAX-(UINT_MAX%l)-1;

    if(config.verbose) {
        int entropy_bits;
        print_char_count("Upper case",
                         config.min_upper_case_chars,
                         config.max_upper_case_chars);
        print_char_count("Lower case",
                         config.min_lower_case_chars,
                         config.max_lower_case_chars);
        print_char_count("Numeric",
                         config.min_number_chars,
                         config.max_number_chars);
        print_char_count("Special",
                         config.min_special_chars,
                         config.max_special_chars);
        printf("Exclude hard-to-reads....: %s\n",
               config.exclude_hard_to_reads ? "yes" : "no");
        printf("Exclude explicitly.......: %s\n",
               config.custom_exclude_chars ?
                   config.custom_exclude_chars : "nothing");
        printf("%s.: %s\n",
               config.custom_chars ?
                   "Explicit character pool." : "Resulting character pool",
               char_pool);
        printf("First character must be..: %s\n",
               config.first_must_be_chars ?
                   config.first_must_be_chars : "no restriction");
        printf("Length of character pool.: %d\n",l);
        printf("Randomizer ceiling.......: %x (%u)\n",ceil,ceil);
        printf("Password length requested: %d\n",config.password_length);
        entropy_bits=config.password_length*log(l)/log(2);
        printf("Entropy..................: %d bits\n",entropy_bits);
        printf("Generated password.......: ");
    }

    for(;; ) {
        count_lower_case_chars=0;
        count_upper_case_chars=0;
        count_number_chars=0;
        count_special_chars=0;
        memset(password,0,sizeof(password));
        p=password;
        for(;; ) {
            getentropy((void*)&tmp,sizeof(tmp));
            if(tmp>ceil) continue;
            m=tmp%l;
            c=char_pool[m];

            if(p==password&&config.first_must_be_chars)
                if(!strchr(config.first_must_be_chars,c))
                    continue;

            if(strchr(lower_case_chars,c)) {
                if(config.min_lower_case_chars!=-1&&
                   (config.min_lower_case_chars>(count_lower_case_chars+1)))
                    continue;
                if(config.max_lower_case_chars!=-1&&
                   (config.max_lower_case_chars<(count_lower_case_chars+1)))
                    continue;
            }

            if(strchr(upper_case_chars,c)) {
                if(config.min_upper_case_chars!=-1&&
                   (config.min_upper_case_chars>(count_upper_case_chars+1)))
                    continue;
                if(config.max_upper_case_chars!=-1&&
                   (config.max_upper_case_chars<(count_upper_case_chars+1)))
                    continue;
            }

            if(strchr(number_chars,c)) {
                if(config.min_number_chars!=-1&&
                   (config.min_number_chars>(count_number_chars+1)))
                    continue;
                if(config.max_number_chars!=-1&&
                   (config.max_number_chars<(count_number_chars+1)))
                    continue;
            }

            if(strchr(special_chars,c)) {
                if(config.min_special_chars!=-1&&
                   (config.min_special_chars>(count_special_chars+1)))
                    continue;
                if(config.max_special_chars!=-1&&
                   (config.max_special_chars<(count_special_chars+1)))
                    continue;
            }

            *p++=c;
            if(strchr(lower_case_chars,c)) ++count_lower_case_chars;
            if(strchr(upper_case_chars,c)) ++count_upper_case_chars;
            if(strchr(number_chars,c)) ++count_number_chars;
            if(strchr(special_chars,c)) ++count_special_chars;
            if(strlen(password)>=config.password_length) break;
        }
        if(config.min_lower_case_chars!=-1&&
           (config.min_lower_case_chars>count_lower_case_chars))
            continue;
        if(config.max_lower_case_chars!=-1&&
           (config.max_lower_case_chars<count_lower_case_chars))
            continue;
        if(config.min_upper_case_chars!=-1&&
           (config.min_upper_case_chars>count_upper_case_chars))
            continue;
        if(config.max_upper_case_chars!=-1&&
           (config.max_upper_case_chars<count_upper_case_chars))
            continue;
        if(config.min_number_chars!=-1&&
           (config.min_number_chars>count_number_chars))
            continue;
        if(config.max_number_chars!=-1&&
           (config.max_number_chars<count_number_chars))
            continue;
        if(config.min_special_chars!=-1&&
           (config.min_special_chars>count_special_chars))
            continue;
        if(config.max_special_chars!=-1&&
           (config.max_special_chars<count_special_chars))
            continue;
        break;
    }

    printf("%s\n",password);

    return(0);
}

//
// read stored options from a file
//

void
get_rc_options(CONFIG* config) {
    FILE* pf;
    char buffer[1024];
    char* pb;
    char* token;
    char* delims="\n\t ";
    pb=getenv("HOME");
    if(pb==NULL) return;
    snprintf(buffer,sizeof(buffer),"%s/.genrc",pb);
    pf=fopen(buffer,"r");
    if(pf==NULL) return;
    for(;; ) {
        if(fgets(buffer,sizeof(buffer),pf)==NULL) break;
        pb=buffer;
        while((token=strsep(&pb,delims))!=NULL) {
            if(token[0]==0) break;
            if(token[0]=='#') break;
            if(token[0]!='-') usage(config);
            switch(token[1]) {
            case 'c':
                config->exclude_hard_to_reads=1;
                break;
            case 'v':
                config->verbose=!config->verbose;
                break;
            case 'u':
                if((token=strsep(&pb,delims))==NULL) usage(config);
                class_count_argv(config,token,
                                 &config->min_upper_case_chars,
                                 &config->max_upper_case_chars);
                break;
            case 'l':
                if((token=strsep(&pb,delims))==NULL) usage(config);
                class_count_argv(config,token,
                                 &config->min_lower_case_chars,
                                 &config->max_lower_case_chars);
                break;
            case 'n':
                if((token=strsep(&pb,delims))==NULL) usage(config);
                class_count_argv(config,token,
                                 &config->min_number_chars,
                                 &config->max_number_chars);
                break;
            case 's':
                if((token=strsep(&pb,delims))==NULL) usage(config);
                class_count_argv(config,token,
                                 &config->min_special_chars,
                                 &config->max_special_chars);
                break;
            case 'L':
                if((token=strsep(&pb,delims))==NULL) usage(config);
                config->password_length=atoi(token);
                break;
            case 'x':
                if((token=strsep(&pb,delims))==NULL)
                    config->hex_only=1;
                else
                    config->hex_only=atoi(token);
                break;
            case 'p':
                if((token=strsep(&pb,delims))==NULL) usage(config);
                config->custom_chars=strdup(token);
                break;
            default:
                usage(config);
            }
        }
    }
}

//
// show usage documentation and exit
//

void
usage(CONFIG* config) {
    char pupper[40];
    char plower[40];
    char pnumber[40];
    char pspecial[40];
    plimit(pupper,sizeof(pupper),
           config->min_upper_case_chars,config->max_upper_case_chars);
    plimit(plower,sizeof(plower),
           config->min_lower_case_chars,config->max_lower_case_chars);
    plimit(pnumber,sizeof(pnumber),
           config->min_number_chars,config->max_number_chars);
    plimit(pspecial,sizeof(pspecial),
           config->min_special_chars,config->max_special_chars);
    printf("Usage:\n"
           "  gen <options>\n"
           "    -u [count-spec]: specify number of upper case characters (default=%s)\n"
           "    -l [count-spec]: specify number of lower case characters (default=%s)\n"
           "    -n [count-spec]: specify number of numeric characters (default=%s)\n"
           "    -s [count-spec]: specify number of special characters (default=%s)\n"
           "    -L: specify password length (default=%d)\n"
           "    -x: hex characters only: 0=no, 1(default)=lower, 2=upper, 3=both (default=%d)\n"
           "    -c: exclude hard-to-read characters from password (like 0 and O) (default=%s)\n"
           "    -p <chars>: specify a custom character pool (default=%s)\n"
           "    -e <chars>: specify a custom set of exclude characters (default=%s)\n"
           "    -f <chars>: specify requirements for first character (default=%s)\n"
           "    -v: show verbose output\n"
           "\nNote:\n"
           "  [count-spec] is specified in \"[min]-[max]\" format.\n",
           pupper,
           plower,
           pnumber,
           pspecial,
           config->password_length,
           config->hex_only,
           (config->exclude_hard_to_reads ? "yes" : "no"),
           (config->custom_chars ? config->custom_chars : "none"),
           (config->custom_exclude_chars ?
               config->custom_exclude_chars : "none"),
           (config->first_must_be_chars ?
               config->first_must_be_chars : "none")
           );
    exit(3);
}

//
// interpret min & max in English
//

char*
plimit(char* b,size_t s,int min,int max) {
    if(min==0&&max==0)
        snprintf(b,s,"none");
    else if(min==-1&&max==-1)
        snprintf(b,s,"unbounded");
    else if(min<=0&&max>=0)
        snprintf(b,s,"no more than %d",max);
    else if(min>=0&&max<=0)
        snprintf(b,s,"no fewer than %d",min);
    else
        snprintf(b,s,"%d-%d",min,max);
    return(b);
}

//
// process class count arguments (min and max delimited by a dash)
//

void
class_count_argv(CONFIG* config,char* argv,int* min,int* max) {
    char* p;
    char* p_min=0;
    char* p_max=0;
    if(!argv) usage(config);
    p_min=argv;
    for(p=argv; *p; p++) {
        if(*p=='-') {
            if(p_max) usage(config);
            *p=0;
            p_max=p+1;
        } else if(*p<'0'||*p>'9') usage(config);
    }
    if(p_min&&*p_min)
        *min=atoi(p_min);
    else
        *min=-1;
    if(p_max)
        if(*p_max)
            *max=atoi(p_max);
        else
            *max=-1;
    else
        *max=*min;
}

//
// interpret and print a set of character count parameters
//

void
print_char_count(char* desc,int min,int max) {
    char s1[100]={0};
    char s2[100]={0};
    char s3[100]={0};
    int l;

    snprintf(s1,sizeof(s1),"%s characters",desc);
    if(min==0&&max==0)
        snprintf(s2,sizeof(s2),"none");
    else {
        if(min==-1)
            snprintf(s2+strlen(s2),sizeof(s2)-strlen(s2),"no-minimum");
        else
            snprintf(s2+strlen(s2),sizeof(s2)-strlen(s2),"min=%d",min);
        snprintf(s2+strlen(s2),sizeof(s2)-strlen(s2),", ");
        if(max==-1)
            snprintf(s2+strlen(s2),sizeof(s2)-strlen(s2),"no-maximum");
        else
            snprintf(s2+strlen(s2),sizeof(s2)-strlen(s2),"max=%d",max);
    }
    memset(s3,'.',sizeof(s3));
    l=min(sizeof(s3)-1,max(0,25-strlen(s1)));
    s3[l]=0;
    printf("%s%s: %s\n",s1,s3,s2);
}

//
// delete exclude characters
//

void
strrmchars(char* st,char* deletes) {
    char* p1;
    char* p2;
    for(p1=st,p2=st; *p1; p1++) {
        if(!strchr(deletes,*p1)) {
            *p2++=*p1;
        }
    }
    *p2=0;
}

//
// remove duplicate characters from a string
//

void
strrmdups(char* st) {
    char seen[256]={0};
    char* p1;
    char* p2;
    for(p1=st,p2=st; *p1; p1++) {
        if(seen[(int)*p1]==0) {
            seen[(int)*p1]=1;
            *p2++=*p1;
        }
    }
    *p2=0;
}
