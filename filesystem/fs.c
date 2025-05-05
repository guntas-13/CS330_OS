#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define BLOCK_SIZE 1024        // 1 KB blocks
#define NUM_BLOCKS 1024        // Total blocks in the disk
#define NUM_INODES 128         // Total inodes
#define MAX_NAME_LEN 32        // Max file name length
#define DISK_FILE "GSFS"  // GUNTAS SARAN'S FILE SYSTEM

// Superblock structure (fits in one block)
typedef struct {
    uint32_t total_blocks;     // Total number of blocks
    uint32_t total_inodes;     // Total number of inodes
    uint32_t free_blocks;      // Number of free blocks
    uint32_t free_inodes;      // Number of free inodes
    uint8_t block_bitmap[NUM_BLOCKS / 8]; // Bitmap for block allocation
    uint8_t inode_bitmap[NUM_INODES / 8]; // Bitmap for inode allocation
    uint8_t reserved[BLOCK_SIZE - 8 * sizeof(uint32_t) - (NUM_BLOCKS / 8) - (NUM_INODES / 8)];
} SuperBlock;

// Inode structure
typedef struct {
    uint32_t size;             // File size in bytes
    time_t timestamp;          // Last modified time
    uint32_t blocks[10];       // Direct block pointers (10 blocks max)
    char name[MAX_NAME_LEN];   // File name
    uint8_t is_used;           // Inode in use flag
} Inode;

// File system structure in memory
typedef struct {
    FILE *disk;                // Disk file pointer
    SuperBlock sb;             // Superblock
    Inode inodes[NUM_INODES];  // Inode table
} FileSystem;

// Initialize the disk file and superblock
void init_filesystem(FileSystem *fs) {
    // Create or open disk file
    fs->disk = fopen(DISK_FILE, "w+b");
    if (!fs->disk) {
        perror("Failed to create disk file");
        exit(1);
    }

    // Initialize superblock
    fs->sb.total_blocks = NUM_BLOCKS;
    fs->sb.total_inodes = NUM_INODES;
    fs->sb.free_blocks = NUM_BLOCKS - 1; // Reserve 1 for superblock
    fs->sb.free_inodes = NUM_INODES;
    memset(fs->sb.block_bitmap, 0, sizeof(fs->sb.block_bitmap));
    memset(fs->sb.inode_bitmap, 0, sizeof(fs->sb.inode_bitmap));
    fs->sb.block_bitmap[0] = 1; // Superblock block is used

    // Write superblock to disk
    fseek(fs->disk, 0, SEEK_SET);
    fwrite(&fs->sb, sizeof(SuperBlock), 1, fs->disk);

    // Initialize inodes
    for (int i = 0; i < NUM_INODES; i++) {
        fs->inodes[i].is_used = 0;
        fs->inodes[i].size = 0;
        memset(fs->inodes[i].blocks, 0, sizeof(fs->inodes[i].blocks));
        fs->inodes[i].name[0] = '\0';
    }

    // Write inodes to disk
    fseek(fs->disk, BLOCK_SIZE, SEEK_SET);
    fwrite(fs->inodes, sizeof(Inode), NUM_INODES, fs->disk);

    fflush(fs->disk);
}

// Load filesystem from disk
void load_filesystem(FileSystem *fs) {
    fs->disk = fopen(DISK_FILE, "r+b");
    if (!fs->disk) {
        perror("Failed to open disk file");
        exit(1);
    }

    // Read superblock
    fseek(fs->disk, 0, SEEK_SET);
    fread(&fs->sb, sizeof(SuperBlock), 1, fs->disk);

    // Read inodes
    fseek(fs->disk, BLOCK_SIZE, SEEK_SET);
    fread(fs->inodes, sizeof(Inode), NUM_INODES, fs->disk);
}

// Allocate a free block
int allocate_block(FileSystem *fs) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        int byte = i / 8;
        int bit = i % 8;
        if (!(fs->sb.block_bitmap[byte] & (1 << bit))) {
            fs->sb.block_bitmap[byte] |= (1 << bit);
            fs->sb.free_blocks--;
            return i;
        }
    }
    return -1; // No free blocks
}

// Allocate a free inode
int allocate_inode(FileSystem *fs) {
    for (int i = 0; i < NUM_INODES; i++) {
        int byte = i / 8;
        int bit = i % 8;
        if (!(fs->sb.inode_bitmap[byte] & (1 << bit))) {
            fs->sb.inode_bitmap[byte] |= (1 << bit);
            fs->sb.free_inodes--;
            fs->inodes[i].is_used = 1;
            return i;
        }
    }
    return -1; // No free inodes
}

// Create a file
int create_file(FileSystem *fs, const char *name, const char *data, uint32_t size) {
    if (strlen(name) >= MAX_NAME_LEN) {
        printf("File name too long\n");
        return -1;
    }

    // Allocate inode
    int inode_idx = allocate_inode(fs);
    if (inode_idx == -1) {
        printf("No free inodes\n");
        return -1;
    }

    Inode *inode = &fs->inodes[inode_idx];
    strncpy(inode->name, name, MAX_NAME_LEN - 1);
    inode->name[MAX_NAME_LEN - 1] = '\0';
    inode->size = size;
    inode->timestamp = time(NULL);

    // Allocate blocks
    int blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (blocks_needed > 10) {
        printf("File too large for inode\n");
        return -1;
    }

    for (int i = 0; i < blocks_needed; i++) {
        int block_idx = allocate_block(fs);
        if (block_idx == -1) {
            printf("No free blocks\n");
            return -1;
        }
        inode->blocks[i] = block_idx;

        // Write data to block
        fseek(fs->disk, block_idx * BLOCK_SIZE, SEEK_SET);
        uint32_t bytes_to_write = (i == blocks_needed - 1) ? (size % BLOCK_SIZE ? size % BLOCK_SIZE : BLOCK_SIZE) : BLOCK_SIZE;
        fwrite(data + i * BLOCK_SIZE, 1, bytes_to_write, fs->disk);
    }

    // Update superblock and inodes on disk
    fseek(fs->disk, 0, SEEK_SET);
    fwrite(&fs->sb, sizeof(SuperBlock), 1, fs->disk);
    fseek(fs->disk, BLOCK_SIZE, SEEK_SET);
    fwrite(fs->inodes, sizeof(Inode), NUM_INODES, fs->disk);
    fflush(fs->disk);

    return 0;
}

// Read a file
void read_file(FileSystem *fs, const char *name) {
    for (int i = 0; i < NUM_INODES; i++) {
        if (fs->inodes[i].is_used && strcmp(fs->inodes[i].name, name) == 0) {
            Inode *inode = &fs->inodes[i];
            char *buffer = malloc(inode->size + 1);
            if (!buffer) {
                printf("Memory allocation failed\n");
                return;
            }
            buffer[inode->size] = '\0';

            uint32_t bytes_read = 0;
            for (int j = 0; j < 10 && inode->blocks[j] != 0; j++) {
                fseek(fs->disk, inode->blocks[j] * BLOCK_SIZE, SEEK_SET);
                uint32_t to_read = (bytes_read + BLOCK_SIZE <= inode->size) ? BLOCK_SIZE : (inode->size - bytes_read);
                fread(buffer + bytes_read, 1, to_read, fs->disk);
                bytes_read += to_read;
            }

            printf("File: %s, Size: %u bytes\nContent:\n%s\n", name, inode->size, buffer);
            free(buffer);
            return;
        }
    }
    printf("File not found: %s\n", name);
}

// Close filesystem
void close_filesystem(FileSystem *fs) {
    if (fs->disk) {
        fclose(fs->disk);
        fs->disk = NULL;
    }
}

int main() {
    FileSystem fs;

    // Initialize and create a new filesystem
    init_filesystem(&fs);

    // Create some files
    const char *data1 = "Hello, this is a test file!\n";
    const char *data2 = "Another file with different content.\n";
    create_file(&fs, "test1.txt", data1, strlen(data1));
    create_file(&fs, "test2.txt", data2, strlen(data2));

    // Close and reload filesystem
    close_filesystem(&fs);
    load_filesystem(&fs);

    // Read files
    read_file(&fs, "test1.txt");
    read_file(&fs, "test2.txt");
    read_file(&fs, "nonexistent.txt");

    // Clean up
    close_filesystem(&fs);

    return 0;
}