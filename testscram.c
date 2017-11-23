/*-
 * Copyright (c) 2017 Joshua Jackson <jjackson@kallisteconsulting.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scram.h"


void get_input_line(char *prompt, char **result) {
    char buf[BUFSIZ] = "";
    char *s;
    printf("%s\n", prompt);
    s = fgets(buf, sizeof (buf) - 1, stdin);
    if (s == NULL) {
        return;
    }
    if (buf[strlen(buf) - 1] == '\n') {
        buf[strlen(buf) - 1] = '\0';
    }
    *result = strdup(buf);
}

void test_client_side_low() {
    char username[] = "user";
    char password[] = "pencil";
    char channel_binding[] = "n,,";
    int r;
    char *client_first;
    char *client_nonce;
    char *server_first;
    char *server_first_decoded;
    char *parsed_combined_nonce;
    char *parsed_server_nonce;
    char *parsed_user_salt;
    int parsed_iteration_count;
    unsigned char *salted_password;
    char *client_final;
    char *server_final;
    r = scram_client_first(username, &client_first, &client_nonce);
    printf("client first message: %s\n", client_first);
    get_input_line("Enter server first message", &server_first);
    r = scram_handle_server_first(server_first, client_nonce, &server_first_decoded, &parsed_combined_nonce, &parsed_server_nonce, &parsed_user_salt, &parsed_iteration_count);
    gen_scram_salted_password(password, parsed_user_salt, parsed_iteration_count, &salted_password);
    r = scram_client_final(server_first_decoded, username, salted_password, client_nonce, parsed_server_nonce, channel_binding, &client_final);
    printf("client final message: %s\n", client_final);
    get_input_line("Enter server final message", &server_final);
    r = scram_handle_server_final(server_final, server_first_decoded, username, salted_password, client_nonce, parsed_server_nonce, channel_binding);
    if (r == SCRAM_OK) {
        printf("Client determines authentication SUCCESS\n");
    }
    else {
        printf("Client determines authentication FAILURE\n");
    }
}

void test_server_side_low() {
    int r;
    char password[] = "pencil";
    char user_salt_b64[] = "QSXCR+Q6sek8bf92";
    char channel_binding[] = "n,,";
    char *client_first;
    char *parsed_username;
    char *client_nonce;
    char *server_first;
    char *server_first_decoded;
    char *server_nonce;
    char *client_final;
    char *server_final;
    unsigned char *salted_password;
    int iterations = 4096;
    get_input_line("Enter client first message", &client_first);
    r = scram_handle_client_first(client_first, &parsed_username, &client_nonce);
    r = scram_server_first(iterations, user_salt_b64, client_nonce, &server_first, &server_first_decoded, &server_nonce);
    printf("server first message: %s\n", server_first);
    get_input_line("Enter client final message", &client_final);
    gen_scram_salted_password(password, user_salt_b64, iterations, &salted_password);
    r = scram_handle_client_final(client_final, server_first_decoded, parsed_username, salted_password, client_nonce, server_nonce);
    if (r == SCRAM_OK) {
        printf("Server determines authentication SUCCESS\n");
        r = scram_server_final(server_first_decoded, parsed_username, salted_password, client_nonce, server_nonce, channel_binding, &server_final);
        printf("server final message: %s\n", server_final);
    }
    else {
        printf("Server determines authentication FAILURE\n");
    }
}

void test_both_sides() {
    char username[] = "user";
    char password[] = "pencil";
    char user_salt_b64[] = "QSXCR+Q6sek8bf92";
    char channel_binding[] = "n,,";
    int iterations = 4096;
    int r;
    //
    char *client_first;
    char *client_nonce;
    //
    char *parsed_username;
    char *parsed_client_nonce;
    //
    char *server_first;
    char *server_first_decoded_client;
    char *server_first_decoded_server;
    char *server_nonce;
    //
    char *parsed_combined_nonce;
    char *parsed_server_nonce;
    char *parsed_user_salt;
    int parsed_iteration_count;
    //
    unsigned char *client_salted_password;
    unsigned char *server_salted_password;
    char *client_final;
    //
    char *server_final;
    //
    r = scram_client_first(username, &client_first, &client_nonce);
    printf("client nonce: %s\n", client_nonce);
    printf("client first message: %s\n", client_first);
    r = scram_handle_client_first(client_first, &parsed_username, &parsed_client_nonce);
    printf("parsed username: %s\n", parsed_username);
    printf("parsed client nonce: %s\n", parsed_client_nonce);
    r = scram_server_first(iterations, user_salt_b64, parsed_client_nonce, &server_first, &server_first_decoded_server, &server_nonce);
    printf("server first message: %s\n", server_first);
    r = scram_handle_server_first(server_first, client_nonce, &server_first_decoded_client, &parsed_combined_nonce, &parsed_server_nonce, &parsed_user_salt, &parsed_iteration_count);
    printf("parsed server nonce: %s\n", parsed_server_nonce);
    printf("parsed combined nonce: %s\n", parsed_combined_nonce);
    printf("parsed user salt: %s\n", parsed_user_salt);
    printf("parsed iteration count: %d\n", parsed_iteration_count);
    gen_scram_salted_password(password, parsed_user_salt, parsed_iteration_count, &client_salted_password);
    r = scram_client_final(server_first_decoded_client, username, client_salted_password, client_nonce, parsed_server_nonce, channel_binding, &client_final);
    printf("client final message: %s\n", client_final);
    gen_scram_salted_password(password, user_salt_b64, parsed_iteration_count, &server_salted_password);
    r = scram_handle_client_final(client_final, server_first_decoded_server, username, server_salted_password, client_nonce, server_nonce);
    if (r == SCRAM_OK) {
        printf("Server determines authentication SUCCESS\n");
        r = scram_server_final(server_first_decoded_server, username, server_salted_password, client_nonce, server_nonce, channel_binding, &server_final);
        printf("server final message: %s\n", server_final);
        r = scram_handle_server_final(server_final, server_first_decoded_client, username, client_salted_password, client_nonce, parsed_server_nonce, channel_binding);
        if (r == SCRAM_OK) {
            printf("Client determines authentication SUCCESS\n");
        }
        else {
            printf("Client determines authentication FAILURE\n");
        }
        free(server_final);
    }
    else {
        printf("Server determines authentication FAILURE\n");
    }
    free(client_first);
    free(client_nonce);
    free(server_first);
    free(server_nonce);
    free(parsed_username);
    free(parsed_client_nonce);
    free(parsed_combined_nonce);
    free(parsed_user_salt);
    free(server_salted_password);
    free(client_salted_password);
}

void help_message() {
    printf("-c to test client side\n");
    printf("-s to test server side\n");
    printf("-b to test both sides\n");
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp("-c", argv[1]) == 0) {
            test_client_side_low();
        }
        else if (strcmp("-s", argv[1]) == 0) {
            test_server_side_low();
        }
        else if (strcmp("-b", argv[1]) == 0) {
            test_both_sides();
        }
        else {
            help_message();
        }
    }
    else {
        help_message();
    }
    return 0;
}
