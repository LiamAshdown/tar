#pragma once

#pragma pack(push, 1)
struct TarHeader {
    char file_name[100];
    char file_mode[8];
    char owner_id[8];
    char group_id[8];
    char file_size[12];
    char last_mod_time[12];
    char checksum[8];
    char type_flag;
    char linked_file_name[100];
    char padding[255];
};
#pragma pack(pop)