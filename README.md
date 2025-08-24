# flv-recovery-ext3-academic

An academic project to recover a deleted FLV file from an Ext3 filesystem.

This project was part of a digital forensics class completed in the spring of 2025. Written in C for a Linux environment.

## Background

This project gives practical application to two areas of study: *filesystem design* and *file carving techniques*. Course curriculum delved into the Ext3 filesystem, while students independently studied a variety of file types. Each student was then tasked with writing a program to recover a deleted file of his or her assigned type from an Ext3 filesystem.

## Written in C

I chose to write this program in C, not C++, as both an exercise in simplicity and an exploration of the dividing line between the two programming languages. This ended up being a highly educational endeavor. I found that working exclusively in C, without the benefit of features like classes or exceptions, required me to adopt an entirely new (or old) paradigm.

The encapsulation and cohesion of classes was sorely missed, and I found myself writing functions in the same unusual style that I have often noticed in older C libraries. That is, functions with too many arguments, uncomfortable names, esoteric effects, complicated returns, roundabout error handling, and unwanted assumptions about state became increasingly likely as the project grew in complexity. After making several design decisions about which of these disadvantages were more or less tolerable, I feel I am more appreciative of modern programming languages and better able to articulate the design challenges they address.

## Ext3 Filesystem

The Ext3 filesystem combines direct and indirect references to data blocks within a file's inode entry. Each inode entry contains 15 pointers. The first 12 of these (0 through 11) point directly to data blocks. For larger files, indirection is used beginning at the 13th pointer (number 12), which points to an indirect block containing many pointers to direct data blocks. The 14th pointer employs double indirection, pointing to an indirect block of indirect block pointers, and the 15th pointer employs triple indirection. Understanding of this system, as well as the particular methods by which metadata pertaining to the filesystem is stored, is central to the design of this program.

## FLV File Format

FLV is an audio and video file format that is no longer widely used. An FLV file contains a header with metadata, followed by a series of audio and video data packets. Each packet also contains metadata, including the size of its data payload and the size of the previous packet. FLV files do not contain end-of-file indicators.

FLV files are particularly prone to corruption because each packet in the file can only be located using the sequence of address information built up by all previous packets. If this information is corrupted at any point in the chain, it's highly likely that an FLV player (or a file recovery program such as this) will be unable to identify any further packets. Therefore, a complete file recovery requires a successful navigation of every single packet in the chain.

## Known Limitations

Not every deleted file can be recovered.

Per assignment specifications, students were permitted to assume certain favorable conditions, such as a freshly formatted Ext3 partition and minimal filesystem activity following file deletion.

Specific methods of locating candidate data blocks for recovery were required as part of the assignment. Possible flaws in these methods were a noteworthy part of classroom discussions but were ultimately dismissed by the professor as unlikely to occur under the conditions permitted and beyond the scope of the project.

For context, this submission and its accompanying live demonstration received a perfect score, including full bonus points for navigating the Ext3 filesystem's use of triple indirection to recover a deleted FLV file larger than 6 GB.

## Compile and Link

A Makefile is provided for a Linux environment with make and g++.

From the project root directory:

```
make
```

To remove the binary and .o files for cleanup or recompilation:

```
make clean
```

## Run

Note that Linux may delay hardware-level I/O operations on a device while simulating to the user that a write or delete has been completed. To ensure that the program is ready to be run, it is recommended to unmount the device from which the file will be recovered. This forces Linux to complete I/O operations on the device.

To run, from the project root directory:

```
./recover path
```

Where *path* is the path to an Ext3 partition from which an FLV file has been deleted.

The program will attempt to recover the file as recovered_file.flv, which will be written to the same directory as the binary.

## Disclaimer

As an academic exercise, this project's scope is limited to a highly specific set of requirements elicited from assignment documentation and classroom discussions. It is presented as a successful fulfillment of learning objectives and a snapshot of my development over time as a programmer.
