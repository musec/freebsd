message Compress {
    required uint32 fd_in=1;
    required uint32 fd_out=2;
    required string original_name=3;
    optional uint32 mtime=4;
}

message Uncompress {
    required uint32 fd_in=1;
    required uint32 fd_out=2;
    required string filename=3;
    optional string pre=4;
    optional uint32 prelen=5;
}

message Return {
    required uint32 size=1;
    required uint32 bytes=2;
}

message Request {
    enum Type { COMPRESS = 1; UNCOMPRESS = 2; }

    required Type type = 1;
    optional Compress compress = 2;
    optional Uncompress uncompress = 3;
}

