
# 构建 Docker 并通过 ssh 使用

## 构建 Docker image

构建 image

```
cd SimpleKernel
docker build -t SimpleKernel-docker .
```

## 启动容器，并配置 ssh

运行以下命令启动容器:

```
docker run --name SimpleKernel-container -itd -p 233:22 -v ./:/home/zone/SimpleKernel SimpleKernel-docker
```

进入 docker container ubuntu 命令行环境

```
docker exec -it SimpleKernel-container /bin/zsh
```

## 配置 ssh

在本地创建 RSA Key:

```
# 检查是否已经存在 RSA key:
ls ~/.ssh
# 如果已经存在 id_rsa.pub 则忽略后续步骤，否则继续
# 将 <comment> 替换为你想要的任意字符串
ssh-keygen -t rsa -b 2048 -C "<comment>"
```

运行 `cat ~/.ssh/id_rsa.pub`, 然后复制内容。

回到容器中

- 默认用户 `zone`:`zone`

```
ssh -p 233 zone@localhost
# 输入密码
```

创建 `/home/zone/.ssh/authorized_keys`，并粘贴。

ssh 登录 ubuntu:

```
ssh -p 233 zone@localhost
```

此时应该能够直接登录，不再需要输入密码

## 配置 vscode

* 安装`Remote - SSH` 插件
* 打开Command Palette: CMD+Shift+P
* 输入`Remote-SSH: Add New SSH Host...`
* 输入SSH命令： `ssh -p 233 zone@localhost`
* 登录成功后，在打开的窗口中打开文件夹。
