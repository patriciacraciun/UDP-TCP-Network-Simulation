#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "buffer.h"
#include "parson.h"
#include <sys/socket.h>

#define HOST "34.246.184.49"
#define PORT 8080

int is_valid(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

void register_user(int sockfd) {
    char username[50], password[50];

    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    if (strchr(username, ' ') || strchr(password, ' ')) {
        printf("ERROR: Username-ul sau parola contine spatii.\n");
        return;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    char *body_data = json_serialize_to_string(root_value);

    char *message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", &body_data, 1, NULL, 0, NULL);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 201 Created") != NULL || strstr(response, "HTTP/1.1 200 OK") != NULL) {
        printf("SUCCESS: Utilizator inregistrat cu succes.\n");
    } else if(strstr(response, "error") != NULL){
        printf("ERROR: Nu s-a putut face inregistrarea.\n");
    }

    json_free_serialized_string(body_data);
    json_value_free(root_value);
    free(message);
    free(response);
}

void login_user(int sockfd, char **cookie) {
    char username[50], password[50];

    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    char *body_data = json_serialize_to_string(root_value);

    char *message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", &body_data, 1, NULL, 0, NULL);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL) {
        char *cookie_start = strstr(response, "Set-Cookie: ");
        if (cookie_start != NULL) {
            cookie_start += 12;
            char *cookie_end = strstr(cookie_start, ";");
            int cookie_length = cookie_end - cookie_start;
            *cookie = (char *)calloc(cookie_length + 1, sizeof(char));
            strncpy(*cookie, cookie_start, cookie_length);
        }

        printf("SUCCESS: Utilizatorul a fost autentificat cu succes\n");
    } else if(strstr(response, "error") != NULL){
        printf("ERROR: Nu s-a putut face autentificarea.\n");
    }

    json_free_serialized_string(body_data);
    json_value_free(root_value);
    free(message);
    free(response);
}

void enter_library(int sockfd, char *cookie, char **token) {
    if (cookie == NULL) {
        printf("ERROR: Utilizatorul nu este autentificat.\n");
        return;
    }

    char *cookies[] = {cookie, NULL};

    char *message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);


    if (strstr(response, "HTTP/1.1 200 OK") != NULL) {
        char *json_start = strstr(response, "\r\n\r\n");
        if (json_start != NULL) {
            json_start += 4;
            JSON_Value *resp_value = json_parse_string(json_start);
            JSON_Object *resp_object = json_value_get_object(resp_value);
            const char *jwt = json_object_get_string(resp_object, "token");
            if (jwt) {
                *token = strdup(jwt);
            }
            json_value_free(resp_value);
        }
        printf("SUCCESS: Utilizatorul are acces la biblioteca\n");
    } else if(strstr(response, "error") != NULL){
        printf("ERROR: Nu s-a putut accesa biblioteca.\n");
    }
    free(message);
    free(response);
}

void get_books(int sockfd, char *cookie, char *token) {
    if (cookie == NULL) {
        printf("ERROR: Utilizatorul nu este autentificat.\n");
        return;
    }

    char *cookies[] = {cookie, NULL};

    char *message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, cookies, 1, token);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);


    if(strstr(response, "error") != NULL){
        printf("ERROR: Nu poti vedea cartile.\n");
        free(message);
        free(response);
        return;
    }

    char *json_start = strstr(response, "\r\n\r\n");
    json_start += 4;

    JSON_Value *resp_value = json_parse_string(json_start);

    JSON_Array *books = json_value_get_array(resp_value);
    size_t count = json_array_get_count(books);

    printf("[\n");
    for (size_t i = 0; i < count; i++) {
        JSON_Object *book = json_array_get_object(books, i);
        if (json_object_has_value_of_type(book, "id", JSONNumber) &&
            json_object_has_value_of_type(book, "title", JSONString)) {
            printf("  {\n");
            printf("    \"id\": %d,\n", (int)json_object_get_number(book, "id"));
            printf("    \"title\": \"%s\"\n", json_object_get_string(book, "title"));
            printf("  }%s\n", i < count - 1 ? "," : "");
        }
    }
    printf("]\n");

    json_value_free(resp_value);
    free(message);
    free(response);
}

void get_book(int sockfd, char *cookie, char *token) {
    if (cookie == NULL) {
        printf("ERROR: Utilizatorul nu este autentificat.\n");
        return;
    }

    int book_id;
    printf("id=");
    scanf("%d", &book_id);
    while (getchar() != '\n');

    char url[100];
    sprintf(url, "/api/v1/tema/library/books/%d", book_id);

    char *cookies[] = {cookie, NULL};
    char *message = compute_get_request(HOST, url, NULL, cookies, 1, token);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if(strstr(response, "error") != NULL){
        printf("ERROR: Nu poti vedea carti.\n");
        free(message);
        free(response);
        return;
    }

    char *json_start = strstr(response, "\r\n\r\n");

    json_start += 4;

    JSON_Value *resp_value = json_parse_string(json_start);

    JSON_Object *book = json_value_get_object(resp_value);

    printf("{\n");
    printf("  \"id\": %d,\n", (int)json_object_get_number(book, "id"));
    printf("  \"title\": \"%s\",\n", json_object_get_string(book, "title"));
    printf("  \"author\": \"%s\",\n", json_object_get_string(book, "author"));
    printf("  \"publisher\": \"%s\",\n", json_object_get_string(book, "publisher"));
    printf("  \"genre\": \"%s\",\n", json_object_get_string(book, "genre"));
    printf("  \"page_count\": %d\n", (int)json_object_get_number(book, "page_count"));
    printf("}\n");

    json_value_free(resp_value);
    free(message);
    free(response);
}

void add_book(int sockfd, char *cookie, char *token) {
    if (cookie == NULL) {
        printf("ERROR: Utilizatorul nu este autentificat.\n");
        return;
    }

    char title[100], author[100], genre[100], publisher[100], page_count_str[10];
    int page_count;

    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0;

    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strcspn(author, "\n")] = 0;

    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;

    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strcspn(publisher, "\n")] = 0;

    printf("page_count=");
    fgets(page_count_str, sizeof(page_count_str), stdin);
    page_count_str[strcspn(page_count_str, "\n")] = 0;

    if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0) {
        printf("ERROR: Tip de date incorect pentru carte.\n");
        return;
    }

    if (!is_valid(page_count_str)) {
        printf("ERROR: Tip de date incorect pentru numarul de pagini.\n");
        return;
    }

    page_count = atoi(page_count_str);
    if (page_count <= 0) {
        printf("ERROR: Numarul de pagini trebuie sa fie un numar pozitiv.\n");
        return;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", page_count);

    char *body_data = json_serialize_to_string(root_value);

    char *cookies[] = {cookie, NULL};
    char *message = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", &body_data, 1, cookies, 1, token);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);


    if(strstr(response, "error") != NULL){
        printf("ERROR: Nu poti sa adaugi carti.\n");
    } else {
        printf("SUCCESS: Cartea a fost adaugata cu succes\n");
    }

    json_free_serialized_string(body_data);
    json_value_free(root_value);
    free(message);
    free(response);
}

void delete_book(int sockfd, char *cookie, char *token) {
    if (cookie == NULL) {
        printf("ERROR: Utilizatorul nu este autentificat.\n");
        return;
    }

    int book_id;
    printf("id=");
    scanf("%d", &book_id);
    while (getchar() != '\n');

    char url[100];
    sprintf(url, "/api/v1/tema/library/books/%d", book_id);

    char *cookies[] = {cookie, NULL};
    char *message = compute_delete_request(HOST, url, NULL, cookies, 1, token);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "error") != NULL) {
        printf("ERROR: Nu poti sa stergi carti.\n");
    } else {
        printf("SUCCESS: Cartea cu id %d a fost stearsa cu succes!\n", book_id);
    }

    free(message);
    free(response);
}


void logout_user(int sockfd, char *cookie) {
    if (cookie == NULL) {
        printf("ERROR: Utilizatorul nu este autentificat.\n");
        return;
    }

    char *cookies[] = {cookie, NULL};
    char *message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL) {
        printf("SUCCESS: Utilizatorul s-a delogat cu succes!\n");
    } else  if(strstr(response, "error") != NULL){
        printf("ERROR: Delogarea nu a functionat.\n");
    }

    free(message);
    free(response);
}

int main() {
    char *cookie = NULL;
    char *token = NULL;
    char command[50];

    while (1) {
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

        if (strcmp(command, "register") == 0) {
            register_user(sockfd);
        } else if (strcmp(command, "login") == 0) {
            login_user(sockfd, &cookie);
        } else if (strcmp(command, "enter_library") == 0) {
            enter_library(sockfd, cookie, &token);
        } else if (strcmp(command, "get_books") == 0) {
            get_books(sockfd, cookie, token);
        } else if (strcmp(command, "get_book") == 0) {
            get_book(sockfd, cookie, token);
        } else if (strcmp(command, "add_book") == 0) {
            add_book(sockfd, cookie, token);
        } else if (strcmp(command, "delete_book") == 0) {
            delete_book(sockfd, cookie, token);
        } else if (strcmp(command, "logout") == 0) {
            logout_user(sockfd, cookie);
            free(cookie);
            free(token);
            cookie = NULL;
            token = NULL;
        } else if (strcmp(command, "exit") == 0) {
            close_connection(sockfd);
            if (cookie != NULL) {
                free(cookie);
            }
            if (token != NULL) {
                free(token);
            }
            break;
        }
        close_connection(sockfd);
    }

    return 0;
}
