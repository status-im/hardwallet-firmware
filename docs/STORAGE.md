# Data storage

The device has a dedicated area for storing data, which is 1/4 of the total device memory. Currently this means 260kb, which is actually rather large.

The device uses flash storage, which is not byte-programmable so we must keep in mind the following properties:

1. The minimum amount of data that can be written at once is a double-word (64 bytes)
2. The size of a flash page is 2k
3. Data cannot be rewritten once programmed, but can be zeroed in double-word chunks
4. Flash pages can only be erased as a whole block (2k at once), this is the only way to reprogram

Frequent erase cycles shorten the lifespan of the memory. Furthermore, writing a page is not an atomic operation, so if part of the data being erased is important, a backup on a second page must happen first.

Although the chip is protected from reading, our security model should not depend on it as the only security barrier. Sensitive data must be encrypted whenever possible.

What we need to store on device is:

1. The master wallet key/chain
2. Data to verify the PIN
3. The counter of wrong PIN insertions
4. Possibly some settings/metadata
5. All keys derived using BIP32. Since these can be re-generated from the master key, this can be considered to be a cache.
6. The list of PIN-less key paths

Of these, only the master wallet cannot be changed during normal usage. Changing it will only possible through a device initialization/restore, which erases the entire storage. The other keys can also not be changed but new ones can be added, and eventually we might need to delete old ones to make place for new ones.

The order in which these are listed is not related to the actual memory layout.

## General storage strategies

1. The value 0xffffffffffffffff is forbidden at the beginning of a piece of information, since this indicates the end of information. Since the data we are handling is not arbitrary this limitation is actually easy to enforce and eventually work around.
2. All structures will be designed to fit an exact number of times within a page, no unit of information can span across pages, so there might be some empty space at the end when an exact size match is not possible.
3. The first 8 bytes (1 double word) of each page is the header. This information will be mostly used for backups and versioning.
4. Erase/rewrite of persistent data (i.e: not cache) is done by first writing the new data to a temporary page. A pool of multiple pages will be used to balance the load on each. The copying scheme must guarantee that partial copies are detected and corrected.
5. since data cannot be overwritten, it will be rewritten every time in the allocated area and only the last value will be considered. When there is no space anymore for this, the data will be compacted to contain only the current value(s) and rewritten. All pages of the same logical type must be rewritten at once in this case, to make it easier to identify free space.
6. cache data cannot be invalidated, meaning the first version of each entry is considered to be the actual one and still be valid. Single entries can be deleted by zeroing them out. When the cache is full, it will be logically rotated. This means older cache entries will be deleted to leave place for new ones. This is not optimal from a performance point of view, since the old cache entries might be very used, but it is easy to implement and the impact will be minimal. Cache will be abundant to avoid having to erase it at all.
7. sensitive data will be zeroed out when a newer value is written.
8. when writing to flash, interrupts must be disabled.

This storage strategy makes reading slower, since you must search the data first. However the performance impact should not be noticeable to the user. Since the we have a relatively abundant memory and small amount of data to write, erase operations should be quite rare.

## Page header

The first 8 bytes (a double word) are used for the page header. The header has the following structure
     
    b   63 - 48      47 - 40       39 - 32       31 - 0
    |======================================================| 
    |    TYPE   |    VERSION    |   INDEX   |  WRITE COUNT |
    |======================================================| 
    
Basically the first word is a magic number, which encodes a page type (2 bytes), a version of the page type, needed if format changes (1 byte), the index of the page, needed if this kind of page is present on several pages (1 bytes). The other 32-bit word is the write count of this page, written as a little-endian integer.

When reading a page the header is verified for correctness. This is useful for detecting partial rewrites (more details will follow).

## Page rewriting

Pages will be rewritten when there is no space to append new information. Since the power might be interrupted or other errors might occur at any point, we must simulate atomic writing. The procedure for rewriting is

1. Locate a free swap page 
2. Write the new data in the swap page but do not write the header
3. Write the header on the first double word of the swap page. The write count must be 1 higher than the target page.
4. Erase the page to be replaced
5. Copy the content of the swap page to the target page, except for the header
6. Copy the header from the swap to the target page
7. Zero the header of the swap page
8. Erase the next swap page if it is free

If the operation is interrupted before step 3 completes, the update will be cancelled but the original data will remain. If step 3 has been completed, the write process will proceed on next boot. If multiple pages must be rewritten for the data to be coherent, step 1-3 must be performed for each page and only then steps 4-7 can be performed. Step 8 is performed only after the last write (i.e: when the next page is free)

If the interruption happens between 6 and 7 the entire header between swap page and target page will be identical (including write count), in this case we will resume from step 7.

A swap page is considered to be free when step 7 has been performed and is usable when the page has been erased. During boot, if no usable swap page is found, the first free page will be erased. This happens after write recovery. This situation only happens if power is lost between step 7 and 8 and there are no usable pages.

## Page layout

On the current hardware, 130 pages are allocated for the user data area. The current layout leaves space for future expansion. Pages with the same logical type (but different index) do not need to be contigous, but of course they will be for the first revision. An exception to this is the swap area, which needs to be contigous and is located at the end of the memory.

    |===================================| 
    |          Write-once data          | Page 0
    |-----------------------------------|
    |             PIN data              | Page 1-2
    |-----------------------------------|
    |                                   |
    |           PIN-less list           | Page 3-6
    |                                   |
    |-----------------------------------|
    |                                   |
    |            Settings               | Page 7-10
    |                                   |
    |-----------------------------------|
    |                                   |
    |                                   |
    |        Derived keys cache         | Page 10-29
    |                                   |
    |-----------------------------------| 
    |                                   |
    |                                   |
    |                                   |
    |                                   |
    |            Reserved               | Page 30-121
    |                                   |
    |                                   | 
    |                                   |
    |                                   |
    |-----------------------------------|   
    |                                   | 
    |                                   |
    |              Swap                 | Page 122-129 
    |                                   |
    |                                   |
    |===================================|

This allocation is preliminary and can change during the development phase. There is a lot of room for future expansion.

## Page description

TODO: describe how each page type is used.

