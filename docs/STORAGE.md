# Data storage

The device has a dedicated area for storing data, which is 1/4 of the total device memory. Currently this means 260kb, which is actually rather large.

The device uses flash storage, which is not byte-programmable so we must keep in mind the following properties:

1. The minimum amount of data that can be written at once is a double-word (64 bits)
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
5. All wallets derived using BIP32. Since these can be re-generated from the master wallet, this can be considered to be a cache.
6. The list of PIN-less wallet paths

Of these, only the master wallet cannot be changed during normal usage. Changing it is only possible through a device initialization/restore, which erases the entire storage. The other keys can also not be changed but new ones can be added, and eventually we might need to delete old ones to make place for new ones.

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

This storage strategy makes reading slower, since you must search the data first. However the performance impact should not be noticeable to the user. Since we have a relatively abundant memory and small amount of data to write, erase operations should be quite rare.

## Page header

The first 8 bytes (a double word) are used for the page header. The header has the following structure
     
    b   63 - 48      47 - 40       39 - 32       31 - 0
    |======================================================| 
    |    INDEX    |   VERSION    |   TYPE   |  WRITE COUNT |
    |======================================================| 
    
Basically the first word is a magic number, which encodes a page type (2 bytes), a version of the page type, needed if format changes (1 byte), the index of the page, needed if this kind of page is present on several pages (1 bytes). The other 32-bit word is the write count of this page, written as a little-endian signed integer (but negative count is not allowed).

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
    |             PIN data              | Page 1
    |-----------------------------------|
    |            Counter area           | Page 2-3
    |-----------------------------------|
    |                                   |
    |        PIN-less wallet list       | Page 4-6
    |                                   |
    |-----------------------------------|
    |                                   |
    |            Settings               | Page 7-9
    |                                   |
    |-----------------------------------|
    |                                   |
    |                                   |
    |        Derived keys cache         | Page 10-39
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

## Security

All sensitive data will be encrypted using AES-128 in CBC mode using a random master key generated during device initialization. This means a 16 byte overhead for the random IV which needs to be stored along the ciphertext.

The master key needs to be stored in a secure area. The protection offered by the STM32L4 is not unbreakable, but the target STM32WB chip offers a "customer key storage" area. We do not have more details on this at the moment, but this is probably an area protected from physical attacks and this is where the master key will be stored. For now it will just be stored in regular flash. This is totally insecure if an attacker somehow manages to bypass read-protection but for the prototype it is fine.

## Write-once data area

__Magic number__: 0x574F0100

The area reserved for write-once data contains data structures of fixed length at predetermined offsets. This data is generated and written during device initialization and is never updated. The first 8 bytes contain a header like all other pages.

    |===================================| 
    |         Magic (0x574F0100)        | byte 0-3
    |-----------------------------------|
    |            Write counter          | byte 4-7
    |-----------------------------------|
    |     Master Wallet Public Key      | byte 8-40
    |-----------------------------------|
    |             Random data           | byte 41-51
    |-----------------------------------|
    |Master Wallet Priv Key (encrypted) | byte 52-83
    |-----------------------------------|
    |      Master Wallet Address        | byte 84-103
    |-----------------------------------|
    |              Reserved             | byte 104-2031
    |-----------------------------------| 
    |  PIN encryption key (TEMPORARY!)  | byte 2016-2041
    |-----------------------------------| 
    | Master encryption key (TEMPORARY!)| byte 2032-2047
    |===================================|

The public key is stored in its compressed form. The IV used for encryption is composed of the last 5 bytes of the public key concatenated to the 11 random bytes following it.


## PIN data area

__Magic number__: 0x504e0100

The PIN data area is used to verify the PIN. The PIN could be stored plain, since if an attacker manages to read out the memory the PIN does not offer any protection. However, since the user might be using the same PIN for other applications, we do not want it to be recoverable. Hashing would be possible, but brute-forcing the hash would be extremely easy due to the small amount of possible PINs and does not even require pre-built tables, so a salt would not help.

For this reason, the PIN will be encrypted using AES-CBC mode using random IV and padded with zeros. To verify PIN correctness, the provided PIN will be padded and encrypted using the same IV as the current PIN. Then byte-wise comparison of the 16-byte block will be performed. All 16 bytes will always be compared to avoid side-channel attacks. We could have saved 16-bytes and the need to generate an IV by using AES in ECB mode. While this would be secure (since we are using a single block), it would reveal patterns like and old PIN being set again, etc.

The structure of a PIN is as follows, bytes 16-31 are encrypted.

    b      0-15            16          17-X       X-31
    |======================================================| 
    |       IV       | PIN length |     PIN   |  Padding   |
    |======================================================| 

Where X is determined by PIN length. The maximum length of the PIN is 15 characters, but it will most likely be between 4 and 6.

When veryfing the PIN, the software must scan the area until it reaches an empty PIN area (i.e: 8 consecutive 0xff bytes or the end of the area). The PIN before the empty area is the current one which must be verified. This way, changing PIN means appending a new PIN after the current one. Since a PIN is 32 bytes long and we have 2040 available bytes, we can change PIN 63 times (leaving a 24 spare bytes at the end). If the PIN is changed again, the page must be rewritten as described in the page rewriting chapter. In this case only the new PIN will be written in the fresh page.

## Counter area

__Magic number__: 0x4354010X

This area provides a place to keep counters. An entry is 8 bytes long and the size of each individual counter can vary from 4 to 64 bits (in 4-bits increments). At the moment, only the PIN retry counter in the lowest 4 bits of the entry is stored (giving a maximum theoretical retry count of 15, but pratically we want to keep it between 3 and 10). Reading and updating the counter entry works exactly as with the PIN entry. Each page can fit exactly 255 updates, but in this case 2 pages are allocated. When rewriting, the current counters value must be the first entry of the first page, while the rest must remain in the erased state.

## PIN-less list

__Magic number__: 0x4e50010X

This is a list of wallet paths which do not require a PIN when signing. Each entry is exactly 40 bytes long, allowing 9 levels of nesting in the hierarchy below the master wallet. The length does not change even if less than 9 levels are used. The structure is as follows

    b   0-3       4-7       8-11     12-15     16-19    20-23     24-27     28-31     32-35     36-39
    |===================================================================================================| 
    |  Count  | Level 1 | Level 2 | Level 3 | Level 4 | Level 5 | Level 6 | Level 7 | Level 8 | Level 9 |
    |===================================================================================================| 

This first word is a counter of how many levels this entry contains (higher ones are to be ignored). If this value is 0xffffffff it means that the entry is empty and can be programmed. If this value is 0x00000000 it means that the entry has been erased and must be ignored.

Adding an entry to the list means finding an empty spot by scanning the pages sequentially and finding the first entry with count value  0xffffffff. Duplicate entries are not allowed. Exactly 51 entries fit on one page. Erasing an entry from the list means overwriting the first double word with zeros.

When the list is full, an attempt must be made to compact the list (i.e: generate a new list without the erased entries) and add the new item. If this is not possible (there are no erased entries), an error condition should be returned. Using 3 pages, the list can be 153 entries long, which is probably way more than needed.

## Settings

__Magic number__: 0x5331010X

Settings work the same as with the counters area. Each entry (size tbd, must be a multiple of 8 bytes) contains a copy of all settings. Should we need to add new settings, a new page type with the additional settings structure will be defined.

At the moment there are no defined settings, so the structure of the settings is not defined either.

## Derived keys cache

__Magic number__: 0x4b4301XX

The cache of derived keys is used to store all keys which have been derived using the algorithms defined in the BIP32 specification. The structure of each entry is composed by a 40-bytes path (as defined in the PIN-less list chapter) followed by a key with the same structure as the master wallet key.

New cache entries can be only appended. When the cache is full, the oldest page will simply be erased and appending will continue from there. The write counter of the header is used in a different manner from other pages, it is a sequence indicating the page order. When the cache is full, the page with the lowest counter will be erased. The fresh page will have a counter which is 1 higher than the previous last page. This allows erasing the cache in a cyclic manner, balancing the load on all pages. The cache does not use page rewriting, so the swap pages are not used.

Each entry is 136 bytes long, meaning we can fit 15 keys in a single page. We have allocated 30 pages meaning 450 keys at once. This should be enough to never fill the cache by normal usage. We have a lot of unallocated pages however, so we might allocate even more pages if needed.

When searching the cache, the code should keep track of the longest match found. This allows, even in case of a cache miss, to at least start key derivation from an intermediate step. When deriving a key, all intermediates should be written to the cache (without duplicates).
