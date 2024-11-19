import datetime
import os
import pathlib
import shutil
import subprocess
import time
import unittest

class Lab4TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.year = 2024
        base_dir = pathlib.Path(__file__).resolve().parent
        build_dir = base_dir.joinpath('build')
        shutil.rmtree(build_dir, ignore_errors=True)
        subprocess.run(['meson', 'setup', 'build'],
                       check=True, cwd=base_dir, stdout=subprocess.DEVNULL)
        subprocess.run(['meson', 'compile', '-C', 'build'],
                       check=True, cwd=base_dir, stdout=subprocess.DEVNULL)
        subprocess.run(['build/ext2-create'], capture_output=True)
        p = subprocess.run(['dumpe2fs', 'hello.img'], capture_output=True, text=True)
        cls.dump_lines = p.stdout.splitlines()
        p = subprocess.run(['fsck.ext2', '-f', '-n', 'hello.img'], capture_output=True, text=True)
        cls.fsck_lines = p.stdout.splitlines()
        p = subprocess.run(['ls', '-f', '-n', 'hello.img'], capture_output=True, text=True)
        cls.ls_lines = p.stdout.splitlines()
        subprocess.run(['mkdir', 'mnt'], capture_output=True)
        subprocess.run(['mount', '-o', 'loop', 'hello.img', 'mnt'])
        cls.root_stat = os.lstat('mnt') if os.path.exists('mnt') else None
        cls.hello_world_stat = os.lstat('mnt/hello-world') if os.path.exists('mnt/hello-world') else None
        cls.hello_stat = os.lstat('mnt/hello') if os.path.exists('mnt/hello') else None

    @classmethod
    def tearDownClass(cls):
        subprocess.run(['umount', 'mnt'])
        subprocess.run(['rmdir', 'mnt'], capture_output=True)
        subprocess.run(['make', 'clean'], capture_output=True)

    def _get_dump_value(self, key):
        for line in self.dump_lines:
            s = line.split(':', 1)
            if len(s) != 2:
                continue
            current_key = s[0].strip()
            if key != current_key:
                continue
            current_value = s[1].strip()
            return current_value
        self.fail(f'dumpe2fs key "{key}" not found')

    def _get_fsck_next_line(self, s):
        i = iter(self.fsck_lines)
        while True:
            try:
                line = next(i)
                if line == s:
                    return next(i)
            except StopIteration:
                break
        self.fail(f'fsck.ext2 line "{s}" not found')

    def test_magic(self):
        self.assertEqual(self._get_dump_value('Filesystem magic number'),
                         '0xEF53')

    def test_revision(self):
        self.assertEqual(self._get_dump_value('Filesystem revision #'),
                         '0 (original)')

    def test_state(self):
        self.assertEqual(self._get_dump_value('Filesystem state'),
                         'clean')

    def test_errors(self):
        self.assertEqual(self._get_dump_value('Errors behavior'),
                         'Continue')

    def test_inode_count(self):
        self.assertEqual(self._get_dump_value('Inode count'),
                         '128')

    def test_block_count(self):
        self.assertEqual(self._get_dump_value('Block count'),
                         '1024')

    def test_reserved_block_count(self):
        self.assertEqual(self._get_dump_value('Reserved block count'),
                         '0')

    def test_free_blocks(self):
        self.assertEqual(self._get_dump_value('Free blocks'),
                         '1000')

    def test_free_inodes(self):
        self.assertEqual(self._get_dump_value('Free inodes'),
                         '115')

    def test_first_block(self):
        self.assertEqual(self._get_dump_value('First block'),
                         '1')

    def test_block_size(self):
        self.assertEqual(self._get_dump_value('Block size'),
                         '1024')

    def test_fragment_size(self):
        self.assertEqual(self._get_dump_value('Fragment size'),
                         '1024')

    def test_blocks_per_group(self):
        self.assertEqual(self._get_dump_value('Blocks per group'),
                         '8192')

    def test_fragments_per_group(self):
        self.assertEqual(self._get_dump_value('Fragments per group'),
                         '8192')

    def test_inodes_per_group(self):
        self.assertEqual(self._get_dump_value('Inodes per group'),
                         '128')

    def test_inode_blocks_per_group(self):
        self.assertEqual(self._get_dump_value('Inode blocks per group'),
                         '16')

    def test_mount_count(self):
        self.assertEqual(self._get_dump_value('Mount count'),
                         '0')

    def test_maximum_mount_count(self):
        self.assertEqual(self._get_dump_value('Maximum mount count'),
                         '-1')

    def test_check_interval(self):
        self.assertEqual(self._get_dump_value('Check interval'),
                         '1 (0:00:01)')

    def test_check_interval(self):
        self.assertEqual(self._get_dump_value('Check interval'),
                         '1 (0:00:01)')
        
    def test_last_write_time(self):
        self.assertIn(str(self.year), self._get_dump_value('Last write time'))
        
    def test_last_checked(self):
        self.assertIn(str(self.year), self._get_dump_value('Last checked'))

    def test_fsck_pass_1(self):
        next_line = self._get_fsck_next_line('Pass 1: Checking inodes, blocks, and sizes')
        self.assertEqual(next_line, 'Pass 2: Checking directory structure')

    def test_fsck_pass_2(self):
        next_line = self._get_fsck_next_line('Pass 2: Checking directory structure')
        self.assertEqual(next_line, 'Pass 3: Checking directory connectivity')

    def test_fsck_pass_3(self):
        next_line = self._get_fsck_next_line('Pass 3: Checking directory connectivity')
        self.assertEqual(next_line, 'Pass 4: Checking reference counts')

    def test_fsck_pass_4(self):
        next_line = self._get_fsck_next_line('Pass 4: Checking reference counts')
        self.assertEqual(next_line, 'Pass 5: Checking group summary information')

    def test_fsck_pass_5(self):
        next_line = self._get_fsck_next_line('Pass 5: Checking group summary information')
        self.assertTrue(next_line.startswith('hello') or next_line == '')

    def test_fsck_summary(self):
        self.assertEqual(self.fsck_lines[-1], 'hello: 13/128 files (0.0% non-contiguous), 24/1024 blocks')

    def test_root_inode(self):
        self.assertEqual(self.root_stat.st_ino, 2)
        
    def test_root_links_count(self):
        self.assertEqual(self.root_stat.st_nlink, 3)

    def test_root_uid(self):
        self.assertEqual(self.root_stat.st_uid, 0)

    def test_root_gid(self):
        self.assertEqual(self.root_stat.st_gid, 0)

    def test_root_mode(self):
        self.assertEqual(self.root_stat.st_mode, 0o40755)

    def test_root_atime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.root_stat.st_atime).year,
                         self.year)

    def test_root_ctime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.root_stat.st_ctime).year,
                         self.year)

    def test_root_mtime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.root_stat.st_mtime).year,
                         self.year)

    def test_hello_world_inode(self):
        self.assertEqual(self.hello_world_stat.st_ino, 12)
        
    def test_hello_world_links_count(self):
        self.assertEqual(self.hello_world_stat.st_nlink, 1)

    def test_hello_world_uid(self):
        self.assertEqual(self.hello_world_stat.st_uid, 1000)

    def test_hello_world_gid(self):
        self.assertEqual(self.hello_world_stat.st_gid, 1000)

    def test_hello_world_mode(self):
        self.assertEqual(self.hello_world_stat.st_mode, 0o100644)

    def test_hello_world_atime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.hello_world_stat.st_atime).year,
                         self.year)

    def test_hello_world_ctime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.hello_world_stat.st_ctime).year,
                         self.year)

    def test_hello_world_mtime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.hello_world_stat.st_mtime).year,
                         self.year)

    def test_hello_inode(self):
        self.assertEqual(self.hello_stat.st_ino, 13)
        
    def test_hello_links_count(self):
        self.assertEqual(self.hello_stat.st_nlink, 1)

    def test_hello_uid(self):
        self.assertEqual(self.hello_stat.st_uid, 1000)

    def test_hello_gid(self):
        self.assertEqual(self.hello_stat.st_gid, 1000)

    def test_hello_mode(self):
        self.assertEqual(self.hello_stat.st_mode, 0o120644)

    def test_hello_atime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.hello_stat.st_atime).year,
                         self.year)

    def test_hello_ctime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.hello_stat.st_ctime).year,
                         self.year)

    def test_hello_mtime(self):
        self.assertEqual(datetime.datetime.fromtimestamp(self.hello_stat.st_mtime).year,
                         self.year)

    def test_hello(self):
        self.assertEqual(os.readlink('mnt/hello'), 'hello-world')

    def test_hello_world(self):
        with open('mnt/hello-world') as f:
            self.assertEqual(f.read(), "Hello world\n")
