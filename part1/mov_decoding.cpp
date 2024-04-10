#define INST_BUF_SIZE 8
int main(int argc, char *argv[]) {
    char inst_buf[INST_BUF_SIZE];
    FILE *fp = fopen(argv[1], "rb");
    size_t num_bytes = fread(inst_buf, sizeof(*inst_buf) * INST_BUF_SIZE, fp);
    for (int i = 0; i < num_bytes; i++) {
        
    }
    return 0;
}