create extension tlsh_gist;

create table test_tlsh(v tlsh);

insert into test_tlsh (v) values ('30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677793');

select v <-> '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' as diff from test_tlsh;
select v <-> NULL  as diff from test_tlsh;


CREATE INDEX tlsh_idx ON test_tlsh USING GIST (v gist_tlsh_ops);

SELECT v, '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' <-> v AS dist
  FROM test_tlsh
  ORDER BY dist LIMIT 1;