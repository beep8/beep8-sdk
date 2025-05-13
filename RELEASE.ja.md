# BEEP-8アプリのリリース

BEEP-8アプリケーションをビルドしたら、beep8.org の BEEP-8 ポータル https://beep8.org に公開して、世界中のプレイヤーと共有できます！

- 各アプリは単一の `.b8` ファイルに対応しています。  
- サンプルアプリのビルド後、ROM は `./obj/<app_name>.b8` に生成されます。  
- `./romfs/instcard.jpg` ファイルはポータルのホームページに表示されるサムネイルとして使用されます。  
  - デフォルトの `instcard.jpg` はプレースホルダーです。アプリに適した画像に置き換えてください。  
<p align="center">
  <img src="docs/img/instcard.png" alt="サムネイル" width="25%" border="1" />
</p>

- サポートされるサムネイル形式: `.jpg`、`.png`、`.gif`、`.mp4`  
    - `.gif` はアニメーションとして表示され、`.mp4` は動画として再生されます。  

- `./run.sh` でビルドするときに、サムネイルデータは `.b8` ファイルに埋め込まれます。  

- サムネイルの解像度はスマートフォンのディスプレイ比率（例: 512×960）に合わせてください。  

- アニメーション付きサムネイルでは、ファイルサイズと画質の観点から `.gif` より `.mp4` の使用を推奨します。  

- `.mp4` を選択する場合、フレームレートは 60fps を推奨します。  

# BEEP-8 DevNet

BEEP-8アプリを公開するには、無料の DevNet アカウントが必要です:

1. [https://beep8.org:3000/](https://beep8.org:3000/) にアクセス  
2. 右上の **サインアップ** をクリック（下図参照）:  
<p align="center">
  <img src="docs/img/devnet_sign_up.png" alt="サインアップ" width="90%" border="1" />
</p>

3. 必要事項を入力して登録を完了します。  
4. ログイン後、ダッシュボードに移動して `.b8` リリースをアップロード・管理します。  
<p align="center">
  <img src="docs/img/devnet_submit.png" alt="ROMを提出" width="90%" border="1" />
</p>

5. **Choose File** をクリックして `./obj/<app_name>.b8` を選択します。  
6. **SUBMIT BEEP-8 ROM (*.b8)** をクリックして ROM をアップロードします。  

7. アップロードが成功すると、以下のような確認ダイアログが表示されます。  
<p align="center">
  <img src="docs/img/devnet_success.png" alt="アップロード成功" width="50%" border="1" />
</p>

8. 提出物はステータス（例: In Review, Developer Rejected, Released）とともにリストに表示されます。ダッシュボードから直接削除や再提出が可能です。  
<p align="center">
  <img src="docs/img/devnet_in_review.png" alt="審査中" width="90%" border="1" />
</p>

9. BEEP-8チームがアプリを審査し、承認されると beep8.org に公開されます。

   審査プロセスでは、以下の最低限のチェックが行われます:  
   - 倫理的または著作権上の問題がないこと  
   - アプリケーションに重大な欠陥がないこと  

---

BEEP-8アプリを公開していただき、ありがとうございます！ご質問やサポートが必要な場合は、[beep8.official@gmail.com](mailto:beep8.official@gmail.com) までメールしてください。

*世界中のプレイヤーと作品を共有してお楽しみください！*
