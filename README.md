# Crack_Reading_sensor
* 구글 스프레드 시트를 새로 생성해주세요.

* *1행 => 1열* 의 이름을 `Timestamp`, *2열* 의 이름을 `Crack`으로 설정해주세요.
* `도구>스크립트 편집기`를 클릭하세요.
* 프로젝트의 이름을 `Form Script`로 바꿔주세요.
* 첨부된 `Script_code`의 내용을 *복사* 해서 `코드.gs`의 내용란에 *덮어써주세요*.
```bash
var SHEET_NAME = "Sheet1";

var SCRIPT_PROP = PropertiesService.getScriptProperties();

function doGet(e){
  return handleResponse(e);
}

function doPost(e){
  return handleResponse(e);
}

function handleResponse(e) {
  var lock = LockService.getPublicLock();
  lock.waitLock(30000); 

  try {
    var doc = SpreadsheetApp.openById(SCRIPT_PROP.getProperty("key"));
    var sheet = doc.getSheetByName(SHEET_NAME);

    var headRow = e.parameter.header_row || 1;
    var headers = sheet.getRange(1, 1, 1, sheet.getLastColumn()).getValues()[0];
    var nextRow = sheet.getLastRow()+1; 
    var row = [];
    for (i in headers){
      if (headers[i] == "Timestamp"){
        row.push(new Date());
      } else {
        row.push(e.parameter[headers[i]]);
      }
    }
    sheet.getRange(nextRow, 1, 1, row.length).setValues([row]);
    return ContentService
          .createTextOutput(JSON.stringify({"result":"success", "row": nextRow}))
          .setMimeType(ContentService.MimeType.JSON);
  } catch(e){
    return ContentService
          .createTextOutput(JSON.stringify({"result":"error", "error": e}))
          .setMimeType(ContentService.MimeType.JSON);
  } finally { 
    lock.releaseLock();
  }
}

function setup() {
    var doc = SpreadsheetApp.getActiveSpreadsheet();
    SCRIPT_PROP.setProperty("key", doc.getId());
}
```
* *코드.gs*의 var SHEET_NAME = "`Sheet1`" 에서 Sheet1을 *구글 스프레드시트의* `시트 이름`과 똑같이 변경해주세요.

* *코드.gs*를 `저장`한 뒤, `실행>함수실행>setup`을 눌러주세요.
* 팝업되는 창에서 권한을 허용해주세요.
* 스크립트 페이지로 돌아와 `파일>버전관리`를 클릭해주세요.
* `Describe what has changed`란에 `Initial version` 을 입력하고 `Save New Version`을 클릭한후, `확인` 버튼을 눌러주세요.
* `수정>현재 프로젝트의 트리거`를 클릭해주세요.
* `새 트리거 만들기`를 클릭한 후, 아래와 같이 설정하고 저장해주세요.
```bash
실행할 함수 선택    #doPost

실행할 배포         #HEAD

이벤트 소스 선택    #스프레드시트에서

이벤트 유형 선택    #양식제출시
```
* 팝업되는 창에서 다시한번 권한을 허용해주세요.

* 스크립트 페이지로 돌아와 `게시>웹 앱으로 배포`를 클릭해주세요.
* 아래와 같이 설정하고 `Deploy`를 클릭해주세요.
```bash
Project Version             #1

Execute the app as me       #Me

Who has access to the app   #Anyone, even anonymous
```
* 팝업된 `URL을 복사`해주세요.
```bash
https://script.google.com/macros/s/AKfycbzBa0I5LuVUNQ3M/exec

# 위와 같은 URL일 것입니다.
```
* `console.particle.io`로 들어가 `Integrations`을 클릭하세요.
```bash
https://console.particle.io/integrations/
```
* `원하는 디바이스`를 클릭한뒤 `EDIT`를 클릭하세요.

* URL에 종전에 복사한 링크로 `덮어쓰기`하고 `SAVE`해주세요.
* 스프레드 시트에 업데이트될 것 입니다.
