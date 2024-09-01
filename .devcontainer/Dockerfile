FROM debian:trixie-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    clangd \
    cmake \
    direnv \
    git \
    locales \
    man-db \
    manpages-dev \
    meson \
    nano \
    npm \
    python3-venv \
    strace \
    vim \
    wget \
    zsh \
    && rm -rf /var/lib/apt/lists/*

RUN sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && locale-gen

RUN sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)" "" --unattended \
    && sed -i 's/ZSH_THEME="robbyrussell"/ZSH_THEME="lukerandall"/g' /root/.zshrc \
    && echo '\n\neval "$(direnv hook zsh)"' >> /root/.zshrc
RUN mkdir -p /root/.config/direnv/ && echo '[global]\nload_dotenv = true\n\n[whitelist]\nprefix = [ "/workspace" ]\n' >> /root/.config/direnv/direnv.toml

CMD ["zsh"]
