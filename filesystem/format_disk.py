import os
import pickle
import time
from dataclasses import dataclass
from typing import List, Set

# Constants
BLOCK_SIZE = 4096  # 4KB blocks
INODE_SIZE = 128   # Size of each inode in bytes
MAX_INODES = 1024  # Initial number of inodes
SUPERBLOCK_OFFSET = 0
INODES_OFFSET = SUPERBLOCK_OFFSET + BLOCK_SIZE
BLOCKS_OFFSET = INODES_OFFSET + (MAX_INODES * INODE_SIZE)

@dataclass
class Superblock:
    """Superblock class to manage filesystem metadata."""
    total_inodes: int
    total_blocks: int
    free_inodes: Set[int]
    free_blocks: Set[int]
    block_size: int
    # magic_number: int = 0xF1LE

    def __init__(self, total_inodes: int, total_blocks: int, block_size: int):
        self.total_inodes = total_inodes
        self.total_blocks = total_blocks
        self.block_size = block_size
        self.free_inodes = set(range(total_inodes))
        self.free_blocks = set(range(total_blocks))

    def allocate_inode(self) -> int:
        if not self.free_inodes:
            raise Exception("No free inodes available")
        return self.free_inodes.pop()

    def free_inode(self, inode_idx: int):
        if inode_idx < 0 or inode_idx >= self.total_inodes:
            raise Exception("Invalid inode index")
        self.free_inodes.add(inode_idx)

    def allocate_block(self) -> int:
        if not self.free_blocks:
            raise Exception("No free blocks available")
        return self.free_blocks.pop()

    def free_block(self, block_idx: int):
        if block_idx < 0 or block_idx >= self.total_blocks:
            raise Exception("Invalid block index")
        self.free_blocks.add(block_idx)

@dataclass
class Inode:
    """Inode class to store file metadata."""
    inode_id: int
    file_type: str
    size: int
    block_count: int
    blocks: List[int]
    ctime: float
    mtime: float
    atime: float
    permissions: int
    owner: str

    def __init__(self, inode_id: int, file_type: str, owner: str = "root"):
        self.inode_id = inode_id
        self.file_type = file_type
        self.size = 0
        self.block_count = 0
        self.blocks = []
        self.ctime = time.time()
        self.mtime = self.ctime
        self.atime = self.ctime
        self.permissions = 0o644 if file_type == "file" else 0o755
        self.owner = owner

class Filesystem:
    """Simple filesystem implementation with read/write functionality."""
    def __init__(self, disk_file: str, total_blocks: int = 1024):
        self.disk_file = disk_file
        self.total_blocks = total_blocks
        self.superblock = None
        self.inodes = [None] * MAX_INODES

        if not os.path.exists(disk_file):
            self._create_filesystem()
        else:
            self._load_filesystem()

    def _create_filesystem(self):
        """Create a new filesystem on the disk file."""
        self.superblock = Superblock(
            total_inodes=MAX_INODES,
            total_blocks=self.total_blocks,
            block_size=BLOCK_SIZE
        )

        root_inode = Inode(inode_id=0, file_type="directory")
        self.inodes[0] = root_inode
        self.superblock.free_inodes.remove(0)

        with open(self.disk_file, "wb") as f:
            # Write superblock
            f.seek(SUPERBLOCK_OFFSET)
            pickle.dump(self.superblock, f, protocol=4)
            f.flush()
            # Write inodes
            f.seek(INODES_OFFSET)
            pickle.dump(self.inodes, f, protocol=4)
            f.flush()
            # Initialize empty blocks
            f.seek(BLOCKS_OFFSET)
            f.write(b'\x00' * (self.total_blocks * BLOCK_SIZE))
            f.flush()

    def _load_filesystem(self):
        """Load filesystem from disk file."""
        try:
            with open(self.disk_file, "rb") as f:
                # Check file size
                f.seek(0, os.SEEK_END)
                file_size = f.tell()
                expected_size = BLOCKS_OFFSET + (self.total_blocks * BLOCK_SIZE)
                if file_size < INODES_OFFSET:
                    raise Exception("Disk file too small to contain inodes")

                # Read superblock
                f.seek(SUPERBLOCK_OFFSET)
                self.superblock = pickle.load(f)

                # Read inodes
                f.seek(INODES_OFFSET)
                self.inodes = pickle.load(f)
        except (pickle.UnpicklingError, EOFError) as e:
            raise Exception(f"Failed to load filesystem: {e}")

    def save_filesystem(self):
        """Save filesystem state to disk."""
        try:
            with open(self.disk_file, "r+b") as f:
                f.seek(SUPERBLOCK_OFFSET)
                pickle.dump(self.superblock, f, protocol=4)
                f.seek(INODES_OFFSET)
                pickle.dump(self.inodes, f, protocol=4)
                f.flush()
        except Exception as e:
            raise Exception(f"Failed to save filesystem: {e}")

    def create_file(self, filename: str, owner: str = "root"):
        """Create a new file with given name."""
        inode_idx = self.superblock.allocate_inode()
        new_inode = Inode(inode_id=inode_idx, file_type="file", owner=owner)
        self.inodes[inode_idx] = new_inode
        self.save_filesystem()
        return inode_idx

    def write_file(self, inode_idx: int, data: bytes):
        """Write data to a file specified by inode index."""
        if inode_idx < 0 or inode_idx >= MAX_INODES or not self.inodes[inode_idx]:
            raise Exception("Invalid inode")
        if self.inodes[inode_idx].file_type != "file":
            raise Exception("Cannot write to non-file inode")

        inode = self.inodes[inode_idx]
        data_size = len(data)
        blocks_needed = (data_size + BLOCK_SIZE - 1) // BLOCK_SIZE

        # Free existing blocks
        for block in inode.blocks:
            self.superblock.free_block(block)
        inode.blocks.clear()
        inode.block_count = 0
        inode.size = 0

        # Allocate new blocks
        try:
            for _ in range(blocks_needed):
                block_idx = self.superblock.allocate_block()
                inode.blocks.append(block_idx)
                inode.block_count += 1
        except Exception as e:
            # Rollback on failure
            for block in inode.blocks:
                self.superblock.free_block(block)
            inode.blocks.clear()
            inode.block_count = 0
            raise e

        # Write data to blocks
        try:
            with open(self.disk_file, "r+b") as f:
                for i, block_idx in enumerate(inode.blocks):
                    offset = BLOCKS_OFFSET + block_idx * BLOCK_SIZE
                    start = i * BLOCK_SIZE
                    end = min(start + BLOCK_SIZE, data_size)
                    block_data = data[start:end]
                    f.seek(offset)
                    f.write(block_data.ljust(BLOCK_SIZE, b'\x00'))
                    f.flush()
        except Exception as e:
            # Rollback on write failure
            for block in inode.blocks:
                self.superblock.free_block(block)
            inode.blocks.clear()
            inode.block_count = 0
            raise Exception(f"Failed to write data: {e}")

        inode.size = data_size
        inode.mtime = time.time()
        inode.atime = inode.mtime
        self.save_filesystem()

    def read_file(self, inode_idx: int) -> bytes:
        """Read data from a file specified by inode index."""
        if inode_idx < 0 or inode_idx >= MAX_INODES or not self.inodes[inode_idx]:
            raise Exception("Invalid inode")
        if self.inodes[inode_idx].file_type != "file":
            raise Exception("Cannot read from non-file inode")

        inode = self.inodes[inode_idx]
        if inode.size == 0:
            return b""

        data = bytearray()
        try:
            with open(self.disk_file, "rb") as f:
                for block_idx in inode.blocks:
                    offset = BLOCKS_OFFSET + block_idx * BLOCK_SIZE
                    f.seek(offset)
                    block_data = f.read(BLOCK_SIZE)
                    data.extend(block_data)
        except Exception as e:
            raise Exception(f"Failed to read data: {e}")

        result = bytes(data[:inode.size])
        inode.atime = time.time()
        self.save_filesystem()
        return result

# Example usage
if __name__ == "__main__":
    # Optionally delete disk.img to force recreation for testing
    # if os.path.exists("disk.img"):
    #     os.remove("disk.img")

    try:
        fs = Filesystem("disk")
        print(f"Superblock: {fs.superblock}")
        print(f"Root inode: {fs.inodes[0]}")

        # Create a new file
        file_inode_idx = fs.create_file("example.txt")
        print(f"Created file with inode: {file_inode_idx}")

        # Write data to the file
        test_data = b"Hello, this is a test file content!"
        fs.write_file(file_inode_idx, test_data)
        print(f"Wrote data to inode {file_inode_idx}")

        # Read data from the file
        read_data = fs.read_file(file_inode_idx)
        print(f"Read data from inode {file_inode_idx}: {read_data.decode()}")

        # Test multi-block write
        large_data = b"A" * (BLOCK_SIZE + 100)
        fs.write_file(file_inode_idx, large_data)
        print(f"Wrote {len(large_data)} bytes to inode {file_inode_idx}")

        # Read it back
        read_large_data = fs.read_file(file_inode_idx)
        print(f"Read {len(read_large_data)} bytes from inode {file_inode_idx}")
        print(f"Data matches: {read_large_data == large_data}")
    except Exception as e:
        print(f"Error: {e}")