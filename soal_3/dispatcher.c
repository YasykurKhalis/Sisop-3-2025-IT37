            if (fd < 0) {
                perror("Gagal membuka file output");
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);

            execlp("cat", "cat", NULL);
            perror("exec gagal");
            exit(1);
        } else {

            close(pipefd[0]);
            dprintf(pipefd[1], "ID,Sender,Destination\n");
            for (int j = 0; j < orders[i].count; j++) {
                dprintf(pipefd[1], "%s\n", orders[i].lines[j]);
            }
            close(pipefd[1]);
            wait(NULL);
        }
    }

    return 0;
}
