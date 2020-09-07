# Crack_Reading_sensor
1. 구글 스프레드 시트를 새로 생성해주세요.
2. 1행 => 1열의 이름을 Timestamp, 2열의 이름을 Crack으로 설정해주세요.
3. 도구>스크립트 편집기를 클릭하세요.
4. 프로젝트의 이름을 Form Script로 바꿔주세요.
5. 첨부된 Script_code의 내용을 복사해서 코드.gs의 내용란에 덮어써주세요.
6. 코드.gs의 var SHEET_NAME = "Sheet1" 에서 Sheet1을 구글 스프레드시트의 시트이름과 똑같이 변경해주세요.
7. 코드.gs를 저장한 뒤, 실행>함수실행>setup을 눌러주세요.
8. 팝업되는 창에서 권한을 허용해주세요.
9. 스크립트 페이지로 돌아와 파일>버전관리를 클릭해주세요.
10. Describe what has changed란에 Initial version 을 입력하고 Save New Version을 클릭한후, '확인' 버튼을 눌러주세요.
11. 수정>현재 프로젝트의 트리거를 클릭해주세요.
12. 새 트리거 만들기를 클릭한 후, doPost/HEAD/스프레드시트에서/양식제출시 로 설정하고 저장해주세요.
13. 팝업되는 창에서 다시한번 권한을 허용해주세요.
14. 스크립트 페이지로 돌아와 게시>웹 앱으로 배포를 클릭해주세요.
15. Project Version:1, Execute the app as:Me, Who has access to the app:Anyone, even anonymous로 설정하고 Deploy를 클릭해주세요.
16. 팝업된 URL을 복사해주세요.
17. console.particle.io로 들어가 Integrations을 클릭하세요.
18. 원하는 디바이스를 클릭한뒤 EDIT를 클릭하세요.
19. URL에 종전에 복사한 링크로 덮어쓰기하고 SAVE해주세요.
20. 스프레드 시트에 업데이트될 것 입니다.