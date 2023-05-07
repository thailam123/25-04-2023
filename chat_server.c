#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

void XoaPhanTu(int arr[], int &n, int index);
bool StartsWith(const char *a, const char *b);
bool valueinarray(int val, int *arr, int n);

int main()
{
   int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (listener == -1)
   {
      perror("socket() failed");
      return 1;
   }

   struct sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(9000);

   if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
   {
      perror("bind() failed");
      return 1;
   }

   if (listen(listener, 5))
   {
      perror("listen() failed");
      return 1;
   }

   fd_set fdread;
   // Mảng clients chứa các client đã kết nối
   int clients[64];
   int numClients = 0;
   // Cấu trúc xác định thời gian chờ (5s)
   struct timeval tv;
   tv.tv_sec = 5;
   tv.tv_usec = 0;

   // gửi câu nhập lại khi sai cú pháp
   char str[] = "sai cú pháp\n";

   char buf[2048];

   // clients đúng cú pháp
   int clients_dung_CP[64];
   int numClients_dung_CP = 0;
   int index = 0;

   while (1)
   {
      FD_ZERO(&fdread);
      FD_SET(listener, &fdread);

      int maxdp = listener + 1;
      for (int i = 0; i < numClients; i++)
      {
         FD_SET(clients[i], &fdread);
         if (clients[i] + 1 > maxdp)
            maxdp = clients[i] + 1;
      }
      // reset timeval struct
      tv.tv_sec = 15;
      tv.tv_usec = 0;
      printf("Waiting for new event.\n");
      int ret = select(maxdp, &fdread, NULL, NULL, NULL);
      if (ret < 0)
      {
         printf("select() failed.\n");
         return 1;
      }
      // quá timeout
      if (ret == 0)
      {
         printf("Timed out.\n");
         continue;
      }
      // check event có kết nối mới
      if (FD_ISSET(listener, &fdread))
      {

         int client = accept(listener, NULL, NULL);
         printf("kết nối thành công: client %d\n", client);
         clients[numClients] = client;
         numClients++;
      }
      // hỏi cho đến khi nhập đúng cú pháp
      for (int i = 0; i < numClients; i++)
         if (FD_ISSET(clients[i], &fdread) && !valueinarray(clients[i], clients_dung_CP, numClients_dung_CP))
         {
            ret = recv(clients[i], buf, sizeof(buf), 0);
            buf[ret] = 0;
            if (StartsWith(buf, "client_id:"))
            {
               // gán giá trị client vào clients_dung_CP
               clients_dung_CP[index] = clients[i];
               index++;
               numClients_dung_CP++;
               printf("nhập đúng cú pháp\n");
            }

            else
            {
               int ret = send(clients[i], str, strlen(str), 0);
               // nhập sai bắt buộc phải nhập lại
               FD_CLR(clients[i], &fdread);
            }
         }
      // check event từ client
      // chuyen sang clients_dung_CP
      for (int i = 0; i < numClients_dung_CP; i++)
         if (FD_ISSET(clients_dung_CP[i], &fdread))
         {
            ret = recv(clients_dung_CP[i], buf, sizeof(buf), 0);
            if (ret <= 0)
            {
               printf("Client %d disconnected\n", clients_dung_CP[i]);
               XoaPhanTu(clients_dung_CP, numClients_dung_CP, i);
               i--;
               continue;
            }
            else
            {
               buf[ret] = 0;
               // printf("Data from client %d: %s\n", clients_dung_CP[i], buf);
               // gửi dữ liệu cho các cilent khác
               for (int i = 0; i < numClients_dung_CP; i++)
               {

                  send(clients_dung_CP[i], buf, strlen(buf), 0);
               }
            }
         }
   }
   close(listener);
}

void XoaPhanTu(int arr[], int &n, int index)
{
   // neu dia chi xoa nho hon 0 thi xoa phan tu dau tien
   if (index < 0)
   {
      index = 0;
   }
   // neu dia chi xoa lon hon hoac bang n thi xoa phan tu cuoi cung
   if (index >= n)
   {
      index = n - 1;
   }
   // Dich chuyen mang ve ben trai tu vi tri xoa
   for (int i = index; i < n - 1; i++)
   {
      arr[i] = arr[i + 1];
   }
   // sau khi xoa giam so luong phan tu mang
   n--;
}

bool StartsWith(const char *a, const char *b)
{
   if (strncmp(a, b, strlen(b)) == 0)
      return 1;
   return 0;
}

bool valueinarray(int val, int *arr, int n)
{
   for (int i = 0; i < n; i++)
   {
      if (arr[i] == val)
         return true;
   }
   return false;
}
