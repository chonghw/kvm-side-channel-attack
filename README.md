# VM간 side channel attack을 통한 RSA 복호화 키 탈취

## covert channel
1. shared memory 영역을 L3캐시 크기만큼 할당 (공유키는 아무값이나)
2. shared memory 영역을 캐시 주소를 구성하는 tag,index,offset 값들을 shared memory 첫 주소에 더하며 캐시 전 영역에 접근(candidate set이라 부름.) 
>2-1. 접근예시) *(char*)((size_t)shm_addr+(tag<<17)+(index<<6)+offset)
>2-2. 이론대로라면 candidate set을 이용하여 conflict set을 만들어야 하지만 현재 시스템에서는 candidate set을 이용해 접근하는 것만으로도 상대 VM의 작업과정을 어느정도 감청가능함.
>2-2. conflict set을 이용해 eviction set이라는 것을 만들어야 한다는데. eviction set의 의미와 활용방안을 모르겠음
3. 캐시셋들을 인덱스 단위로 수신타임 체크
4. 기준치보다 높은 수신타임을 가지는 캐시셋은 상대 VM이 사용하고 있는 캐시영역이라 가정

## side channel
1. RSA복호화 과정에서는 반드시 square & multiple 이 일어날것임.
2. covert channel을 이용해서 기준치보다 높은 캐시셋들이 등장하는 시간을 덤프로 만들어 분석 (타이밍어택)
3. 특정 캐시셋이 특정 시간간격으로 등장한다고 하면 등장하는 순간을 1, 등장하지 않는 순간을 0으로 하여 이를 RSA 키쌍을 만들어내는 p,q의 길이만큼 수집하면 p또는 q값 둘중 하나를 알아낼 수 있게 됨. 
>3-1. 어떤 순간이 q와 p를 알아낼 첫 비트인지 알아내기 난해함. 
>3-2. 어떤 시간단위로 1과 0을 나눌것인지도 모호함.

## slicing(?) window
1. 아직 잘 모르겠군

