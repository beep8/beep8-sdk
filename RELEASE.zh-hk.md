# 發佈 BEEP-8 應用

當你完成 BEEP-8 應用的構建後，就可以將它發佈到 BEEP-8 門戶 https://beep8.org ，與全球玩家分享！

- 每個應用對應一個 `.b8` 檔案。  
- 構建範例應用後，ROM 檔案會生成於 `./obj/<app_name>.b8`。  
- `./romfs/instcard.jpg` 檔案會作為門戶首頁顯示的縮略圖。  
  - 預設的 `instcard.jpg` 只是佔位圖。請替換成適合你應用的圖片。  
<p align="center">
  <img src="docs/img/instcard.png" alt="縮略圖" width="25%" border="1" />
</p>

- 支援的縮略圖格式：`.jpg`、`.png`、`.gif`、`.mp4`  
  - `.gif` 會以動畫形式顯示，`.mp4` 會播放為影片。

- 使用 `./run.sh` 構建時，縮略圖資料會嵌入到 `.b8` 檔案中。

- 縮略圖的解析度應符合智能手機顯示比例（例如 512×960）。

- 若要做動畫縮略圖，建議使用 `.mp4` 而非 `.gif`，以獲得更佳的檔案大小與畫質。

- 選擇 `.mp4` 時，建議使用 60 fps 的畫面更新率。

# BEEP-8 DevNet

要發佈你的 BEEP-8 應用，需要先創建一個免費的 DevNet 帳號：

1. 前往 [https://beep8.org:3000/](https://beep8.org:3000/)  
2. 點擊右上角的 **Sign up**（如圖所示）：  
<p align="center">
  <img src="docs/img/devnet_sign_up.png" alt="註冊" width="90%" border="1" />
</p>

3. 填寫資料並提交完成註冊。  
4. 登入後，進入你的儀表板以上傳並管理 `.b8` 發佈檔。  
<p align="center">
  <img src="docs/img/devnet_submit.png" alt="提交 ROM" width="90%" border="1" />
</p>

5. 點擊 **Choose File**，選擇你的 `./obj/<app_name>.b8`。  
6. 點擊 **SUBMIT BEEP-8 ROM (*.b8)** 上傳 ROM。

7. 上傳成功後，你會看到如下確認視窗：  
<p align="center">
  <img src="docs/img/devnet_success.png" alt="上傳成功" width="50%" border="1" />
</p>

8. 你的提交會以當前狀態（如 In Review、Developer Rejected、Released）列表顯示，你可以在儀表板上直接刪除或重新提交。  
<p align="center">
  <img src="docs/img/devnet_in_review.png" alt="審核中" width="90%" border="1" />
</p>

9. BEEP-8 團隊會審核你的應用；通過後就會在 beep8.org 上正式發佈。

   審核流程包含以下最低限度檢查：  
   - 無倫理或版權問題  
   - 應用不含重大缺陷  

---

感謝你發佈 BEEP-8 應用！如有任何問題或需要支援，請電郵至 [beep8.official@gmail.com](mailto:beep8.official@gmail.com)。

*祝你與全球玩家共享創作愉快！*  
