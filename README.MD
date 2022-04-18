tlsh_gist
---------
A PostgreSQL plugin for tlsh hash, whitch is a fuzzy matching program and library,  Similar files will have similar hash values which allows for the detection of similar objects by comparing their hash values TLSH

see: https://tlsh.org/

doc: https://zhuanlan.zhihu.com/p/497732848


### build


        git clone https://github.com/trendmicro/tlsh.git
        cd tlsh
        mkdir build
        cd build
        cmake .. -D TLSH_SHARED_LIBRARY=1
        make && make install

        # make sure that pg_config is in PATH
        git clone https://github.com/jinyyu/tlsh_gist.git
        cd tlsh_gist
        USE_PGXS=1 make
        USE_PGXS=1 make install

### usage

        create extension tlsh_gist;
        create table test_tlsh(v tlsh);
        CREATE INDEX tlsh_idx ON test_tlsh USING GIST (v gist_tlsh_ops);

        -- index scan
        SELECT v, '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' <-> v AS dist FROM test_tlsh ORDER BY dist LIMIT 1;

        --seq scan
        select v, tlsh_dist(v, '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795') as dist FROM test_tlsh ORDER BY dist LIMIT 1;

        -- % operator
        select v from test_tlsh where v % '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795';



