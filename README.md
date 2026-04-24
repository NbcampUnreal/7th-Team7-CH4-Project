# 🕵️‍♂️ Vigilant (비질란트)

<img width="4400" height="2433" alt="VigilantTitle" src="https://github.com/user-attachments/assets/c60c2b04-129f-42bb-8430-571074273d5d" />


**Vigilant**는 로우폴리 아트 스타일의 중세 시대 테마를 배경으로 한 5~6인 멀티플레이 소셜 디덕션 액션 게임입니다.
 플레이어들은 시민과 마피아로 나뉘어 최종 전투를 대비한 전략적 빌드업을 진행하며 심리전과 액션이 결합된 독특한 플레이 경험을 제공합니다.

 👉 **[🎮 Vigilant 플레이 버전 (Google Drive)](https://drive.google.com/file/d/16B5O1cNeGgHV-jiIZAlVKOlopApA8FGq/view?usp=drive_link)**

---

## 📖 게임 개요
- **프로젝트명**: Vigilant (비질란트)
- **장르**: 퍼즐 방탈출 + 마피아 + 액션
- **참가 인원**: 5~6인 멀티플레이
- **플레이 타임**: 약 10분 + a (캐주얼 지향)
- **아트 컨셉**: 로우폴리(Low-Poly) 에셋 활용 / 기본 테마: 흡혈귀(뱀파이어) 및 중세풍
- **핵심 컨셉**: 최후의 전투를 대비하여 자신을 강화하거나 상대방을 약화시키는 전략적 빌드업
- **승리 목표**
  - **시민**: 미션 수행 및 아이템 파밍 ➔ 탈출 후 투표로 마피아 검거 ➔ 최종전 승리
  - **마피아**: 시민 탈출 지연 유도 후 최종전 승리 (시간이 지날수록 마피아 본체 강화)

---

## 🎥 시연 영상
[![Vigilant 시연 영상](https://img.youtube.com/vi/RGXD9U3Qdgs/0.jpg)](https://www.youtube.com/watch?v=RGXD9U3Qdgs)

---

##  팀 소개
| 이름 | 역할 | 담당 업무 |
| :--- | :--- | :--- |
| **김형백** | UI | 전역 UI 매니저, 투표 시스템 UI, HUD 및 전투 피드백 연출 |
| **이용호** | 게임 시스템 | 게임 코어 아키텍처 설계, 네트워크 동기화 시스템 구현, 페이즈 별 규칙 구현 |
| **정찬호** | 미션 | 퍼즐/미션 서브시스템, 상호작용 기믹 및 환경 오브젝트 로직 |
| **하상빈** | 보스캐릭터 | 보스 액션 및 전투, 보스 전용 스킬, 플레이어 장비 로직 구현 |
| **이현진** | 레벨디자인 | 레벨 디자인, 환경 구성 및 환경 상호작용 배치 |
| **장유진** | 무기, 전투 | 캐릭터 기본 및 전투 액션, 여러 종류의 무기와 각 무기의 애니메이션 제작 |

---

## 💾 주요 시스템 및 기능

### 🛠 스택 기반 페이즈 관리 (Phase Stack System)
- **유연한 흐름 제어**: 미션 중 '막고라(Duel)' 등 돌발 상황 발생 시 기존 페이즈를 일시정지(`Pause`)하고 위에 쌓은(`Push`) 뒤, 종료 시 제거(`Pop`)하여 복귀하는 스택 구조를 도입했습니다.
- **독립적 규칙 설계**: 각 페이즈 객체(`PhaseBase`)는 고유의 승리 조건과 규칙을 가지며, `GameMode`는 이를 참조하여 판정을 수행합니다.

### 📡 서버 중심의 네트워크 아키텍처 (Server-Authoritative)
- **신뢰할 수 있는 판정**: "결정은 서버(GameMode)가 한다"는 원칙에 따라 클라이언트의 입력(RPC)을 서버가 검증하고 결과를 통보하는 구조입니다.
- **이벤트 기반 동기화**: `RepNotify`와 델리게이트를 결합하여, 네트워크 지연 환경에서도 데이터 수신이 확정된 시점에만 UI를 갱신합니다.

### ⚔️ 전략적 빌드업 및 최종 전투
- **성장 및 약화**: 마피아는 시민의 탈출 지연 시간에 비례해 강해지며 시민은 미션 수행과 투표를 통해 마피아를 약화시켜야 합니다.
- **인벤토리리스 루팅**: 별도의 가방 창 없이 상호작용 즉시 무기를 장착/교체하는 직관적인 시스템을 제공합니다.

---

## 🎮 게임 흐름도
```
                       [ Duel (막고라) ]
                         ▲            │ (복귀)
               (일시정지) │            ▼
[ Title ]  ────▶ [ Lobby ] ────▶ [ Mission ] ────▶ [ Vote ] ────▶ [ FinalCombat ] ────▶ [ Ending ]
                     ▲                                                                          │
                     └──────────────────────────────────────(루프 반복)  ────────────────────────┘
```
1. **Lobby Phase**
   - 플레이어 접속 및 '레디' 상태 동기화
   - 전원 레디 시 직업(시민/마피아) 할당 후 게임 시작
2. **Mission Phase**
   - 시민: 퍼즐 해결(탈출 시간 단축) 및 필드 무기 파밍
   - 마피아: 시민 방해 및 탈출 지연 유도
3. **Duel Phase**
   - 특정 조건 달성 시 미션 일시정지(Pause) 후 1:1 전투 진행
   - 전투 종료 시 다시 미션 페이즈로 복귀
4. **Vote Phase**
   - 미션 페이즈 제한 시간 종료 시 투표창 활성화
   - 최다 득표자 색출 및 투표 결과에 연동 시네마틱 재생
5. **Combat Phase**
   - 투표 및 미션 결과(너프/버프 수치)를 스탯에 반영
   - 마피아 본체(보스 폼) vs 시민 다수의 최후의 결전
6. **Ending Phase**
   - 승리 팀 확정 및 전투 결과에 연동된 엔딩 시네마틱 재생
   - 서버 환경 초기화 및 레벨 재시작 (Seamless Travel)
   
---

## 📁 프로젝트 구조
```
Source/Vigilant/
├── Core/        # 게임 시스템, 페이즈 제어, 네트워크 판정 및 룰
├── Character/   # 시민/마피아 보스 베이스 및 전용 로직
├── Weapon/      # 무기 및 장착 아이템 클래스
├── Mission/     # 퍼즐, 미션 오브젝트 및 미션 서브시스템
└── UI/          # UI 매니저 서브시스템 및 각종 위젯
```

---



## 📖 미션
---

### 클래스 구조
미션은 월드 서브시스템을 상속한 미션 서브시스템과 미션.기믹. 아이템의 세 레이어로 분리하여 설계했습니다.
미션 서브시스템이 미션의 전체 달성률, 각 미션의 상태 변화를 외부에 알리는 역할을 담당하도록 구현하였고,
미션은 기믹/아이템 조합을 교체하는 방식으로 구현되었습니다.

<img width="800" height="1020" alt="mission_class_hierarchy_v5 (1)" src="https://github.com/user-attachments/assets/ae242581-04b5-451e-b67f-39f00ec46c49" />

---

### 현재 배치된 미션
현재 게임에는 8종의 미션이 배치되어 있으며 각 미션은 다음과 같은 흐름을 가집니다.

<img width="982" height="762" alt="스크린샷 2026-04-23 195853" src="https://github.com/user-attachments/assets/aa8a8c6d-f66d-4850-be79-e8de231163aa" />

---

### 기믹 (Gimmick)

기믹은 미션을 구성하는 상호작용 오브젝트입니다. 모든 기믹은 `AVGMissionGimmickBase`를 상속하며,
`Inactive → Active → Completed` 세 단계의 상태를 GameplayTag로 관리합니다.
상태는 서버에서만 변경되고 `ReplicatedUsing`을 통해 클라이언트에 동기화됩니다.

현재 구현된 기믹 종류는 다음과 같습니다.

| 기믹 | 클래스 | 설명 |
|------|--------|------|
| 레버 | `AVGMissionGimmickLever` | 상호작용 시 On/Off를 토글합니다. `bIsOneWay` 옵션으로 일방향 전환(예: 횃불)도 지원합니다. 타임라인으로 레버 회전 애니메이션을 처리합니다. |
| 압력판 | `AVGMissionGimmickPressure` | BoxComponent로 Overlap을 감지하며, 필요 인원 수(`RequiredActorCount`)를 채워야 활성화됩니다. `bToggleMode` / `bRevertOnRelease` 옵션으로 다양한 동작 방식을 지원합니다. |
| 석상 | `AVGMissionGimmickStatue` | 상호작용할 때마다 지정된 각도(`RotateStep`)만큼 Yaw 회전합니다. 정답 각도(`AnswerAngle`)에 도달하면 완료 판정을 내립니다. 회전은 `RInterpConstantTo`로 부드럽게 처리하며, Tick은 회전 중에만 활성화합니다. |
| 상자 | `AVGMissionGimmickChest` | 열쇠 아이템을 들고 상호작용하면 열립니다. 타임라인으로 뚜껑 열림 애니메이션을 처리하고, 열림 완료 후 타이머 기반 디졸브 효과로 사라집니다. |
| 제단 | `AVGMissionGimmickAltar` | 여러 슬롯에 지정된 아이템을 각각 올려놓아야 완료됩니다. uint8 비트마스크(`PlacedSlotMask`)로 슬롯 점유 여부를 관리하며, 빈 슬롯에는 Niagara 힌트 이펙트가 표시됩니다. |

---

### 미션 (Mission)

미션은 `AVGMissionBase`를 상속하며, 에디터에서 기믹과 아이템을 조합해 미션의 클리어 조건을 정의합니다.
기믹의 상태 변화 이벤트(`OnGimmickStateChanged`)를 구독하여 클리어 여부를 판정합니다.

현재 구현된 미션 종류는 다음과 같습니다.

| 미션 | 클래스 | 클리어 조건 |
|------|--------|------------|
| 모든 기믹 완료 | `AVGMissionAllGimmicksClear` | 등록된 모든 기믹이 `Completed` 상태가 되면 클리어됩니다. |
| 레버 콤보 | `AVGMissionLeverCombo` | 지정된 레버 인덱스 조합(`RequiredOnLeverIndexes`)이 동시에 활성화되면 클리어됩니다. |
| 압력판 순서 | `AVGMissionPressureSequence` | 등록된 압력판을 셔플된 순서대로 밟아야 합니다. 순서가 틀리면 전체 초기화됩니다. |
| 석상 정렬 | `AVGMissionRotatingStatue` | 등록된 모든 석상이 각자의 정답 각도에 도달하면 클리어됩니다. |
| 전투 | `AVGMissionCombat` | 등록된 모든 샌드백을 처치하면 클리어됩니다. |
| 제한시간 전투 | `AVGMissionTimedCombat` | 샌드백을 처음 공격하면 타이머가 시작되며, 제한시간 내에 모든 샌드백을 처치해야 합니다. |
| 제한시간 동시 활성 | `AVGMissionTimedAll` | 첫 기믹 활성화 시 타이머가 시작되며, 제한시간 내에 모든 기믹을 동시에 활성화해야 합니다. |

---

### 아이템 (Mission Item)

미션 아이템은 `AVGMissionItemBase`를 상속하며, 월드에 배치되어 플레이어가 줍고 기믹에 사용하는 오브젝트입니다.
`Inactive → Carried → Placed / Used` 단계의 상태를 GameplayTag로 관리합니다.

현재 구현된 아이템 종류는 다음과 같습니다.

| 아이템 | 클래스 | 설명 |
|--------|--------|------|
| 열쇠 | `AVGMissionItemKey` | 상자 기믹에 사용합니다. 사용 후 `ItemUsed` 상태로 전환되며 즉시 소멸합니다. |
| 운반 아이템 | `AVGMissionItemCarry` | 제단 기믹의 슬롯에 올려놓는 아이템입니다. 배치 시 기믹 액터에 Attach되며, 배치 정보(`FVGCarryPlaceInfo`)를 Replicate해 모든 클라이언트에 동기화합니다. |

---

### 미션 서브시스템 (VGMissionSubsystem)

`UWorldSubsystem`을 상속한 서브시스템이 미션 전체를 관리합니다.

- 서버는 `Server_RegisterMission`으로 미션을 등록하고 완료 이벤트를 구독합니다.
- 클라이언트는 `Client_RegisterMission`으로 등록하여 UI 갱신 이벤트(`OnMissionRegistered`)를 수신합니다.
- `GetMissionProgress()`로 전체 미션 달성률(완료 수 / 전체 수)을 O(1)로 조회할 수 있습니다.
- MissionID 중복 및 유효성 검증을 등록 시점에 수행하여 런타임 오류를 방지합니다.

---

### 네트워크 설계 원칙

- **상태 변경은 서버 전용** — 모든 기믹·아이템 상태 변경은 `HasAuthority()` 체크 후 수행됩니다.
- **클라이언트 동기화는 ReplicatedUsing** — `OnRep_` 콜백에서 시각 피드백(머티리얼 색상, 애니메이션, VFX)을 처리합니다.
- **서버는 OnRep를 직접 호출** — 서버는 `ReplicatedUsing` 콜백이 자동 실행되지 않으므로 상태 변경 직후 수동으로 호출합니다.

---

### 샌드백 (Sandbag)

전투 미션에 사용되는 타격 오브젝트입니다. `AVGMissionSandbag`과 이를 상속한 `AVGMissionCounterSandbag` 두 종류가 있습니다.

- **AVGMissionSandbag** — `VGStatComponent`로 HP를 관리하며, HP 비율을 Replicate해 클라이언트 UI에 표시합니다. 처치 시 `OnSandbagDefeated` 이벤트로 막타를 친 플레이어 정보를 미션에 전달합니다.
- **AVGMissionCounterSandbag** — 일정 횟수 피격 시 반격 동작을 수행합니다. `Idle → Countering → Hitting → Returning` 단계를 열거형(`EVGSandbagCounterState`)으로 관리하며, 회전·숙이기 애니메이션과 넉백 판정을 포함합니다.
