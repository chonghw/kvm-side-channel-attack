# kvm-side-channel-attack
Side channel attack on KVM Hypervisor
1.하나의 프로세스에서 clflush 호출 유무에 따른 데이터 액세스 타임 측정   O
2.receiver 프로세스에서 clflush 호출로 LLC를 비우고 sender 프로세스에서 일정작업 반복으로 conflict set 추정
3.conflict set 을 기반으로 side channel attack 시도
