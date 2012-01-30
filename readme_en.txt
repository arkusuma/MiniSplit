MiniSplit 1.3
=============

About
-----
MiniSplit is a program used for splitting large file into smaller pieces.
It was designed to be small in size, but without lost in overall performance.
This program was best suited if you want to transfer large file using diskette
or other storage device with limited capacity.

You can use and distribute MiniSplit freely without any charge, as long as you
don't make any change to the original package. This program is provide `as is'.
The author cannot be held responsible for any lost of data resulting from the
use of this program.

Features
--------
  o Error checking using CRC-32, you can pinpoint damaged file part
  o Split a file based on file size or number of file parts
  o Drag-and-drop files from desktop or Windows Explorer
  o Small executable file size (9.5 kb)
  o You don't need MiniSplit to join file parts (see below)

How To Use
----------
Splitting a file:
  o Select the file using one of the following methods:
    (1) type the file name in its full path;
    (2) click browse and select the file;
    (3) drag the file from desktop or Windows Explorer into MiniSplit
  o If needed, enter destination folder
  o Specify the maximum file size (default is: 1420 kbytes). You can also type
    n parts, to split the file into n pieces.
  o If needed, select Split on Operation section
  o Click `Do It'. MiniSplit will ask you for confirmation to start splitting.
  o Files produced will be named as fullname.000, fullname.001, ... and
    fullname.crc (contains error checking codes).

Joining files:
  o Make sure that you have all file parts.
  o Select one of file parts using methods described in splitting section.
  o Select destination folder.
  o Click `Do It'. MiniSplit will ask you for confirmation to start joining.
    If there're corrupted file, MiniSplit will give you the list.

Tips & Tricks
-------------
  o You can drag and drop file to MiniSplit for easier use.
  o You can join splitted files without using MiniSplit:
    On DOS:   copy /b foo.000+foo.001+.... foo
              foo.000, foo.001, ... is the name for all file parts, and foo
              is destination file.
    On Linux: cat foo.[0-9]* > foo

Technical Information
---------------------
Crc file is a binary file used for checking file integrity. It contains:
  o The first 8 bytes is the original file size.
  o n*(4 bytes) next contains CRC-32 of each file parts.

Contact
-------
To make this program better, please notify us for bugs or comments
  mailto:arkusuma@lzesoftware.com

Please visit our website for updates and other programs
  http://www.lzesoftware.com/

==============================================
 MiniSplit Copyright © 2002-2004 LZE Software
==============================================