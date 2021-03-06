create extension tlsh_gist;

select '1416333B50A89B0BB8DED2FC7DF6089B59548AA14B90ED814341DEBF70C9B44CD443AE'::tlsh <-> 'CED022BA04220094C02986B0C8B708C26E529065828BA0A04869A2F2DB060EDBE8E902'::tlsh;

create table test_tlsh(v tlsh);

insert into test_tlsh (v) values ('30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677793');

select v <-> '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' as diff from test_tlsh;
select v <-> NULL  as diff from test_tlsh;


CREATE INDEX tlsh_idx ON test_tlsh USING GIST (v gist_tlsh_ops);


-- index scan
SELECT v, '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' <-> v AS dist FROM test_tlsh ORDER BY dist LIMIT 1;

--seq scan
select v, tlsh_dist(v, '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795') as dist FROM test_tlsh ORDER BY dist LIMIT 1; 


-- % operator
select v, v <-> '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' as dist from test_tlsh where v % '30e5be217761c8b1c27213314d18b62a58bdbdf02b35d2ababe9352c5e780c1b677795' order by dist asc;


