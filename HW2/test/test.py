#!/usr/bin/python3

import os
import subprocess
import sys
import textwrap
from argparse import ArgumentParser

import colorama


class Grader:

    basic_case_dir = "./basic_cases"
    basic_cases = {
        1 : "decl",
        2 : "expr1",
        3 : "expr2",
        4 : "expr3",
        5: "function1",
        6: "function2",
        7: "relation",
        8: "simple",
        9: "statement",
        10: "whilefor1",
        11: "whilefor2"
    }
    basic_case_scores = [0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4]

    advance_case_dir = "./advance_cases"
    advance_cases = {
        1 : "arrayErr",
        2 : "assignErr",
        3 : "compoundErr",
        4 : "conditionErr",
        5 : "declErr",
        6: "funcErr",
        7: "general1",
        8: "general2",
        9: "general3",
        10: "general4",
        11: "general5",
        12: "parentheses",
        13: "syntacticErr",
        14: "whileErr",
        15: "consecutive1",
        16: "consecutive2"
    }
    advance_case_scores = [0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3]

    diff_result = ""

    def __init__(self, parser):
        self.parser = parser

        self.output_dir = "result"
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)

    def get_case_id_list(self, basic_id, advance_id):
        if basic_id == 0:
            self.basic_id_list = self.basic_cases.keys()
        else:
            if not basic_id in self.basic_cases:
                print("ERROR: Invalid basic case ID %d" % basic_id)
                exit(1)
            self.basic_id_list = [basic_id]
        if advance_id == 0:
            self.advance_id_list = self.advance_cases.keys()
        else:
            if not advance_id in self.advance_cases:
                print("ERROR: Invalid advance case ID %d" % advance_id)
                exit(1)
            self.advance_id_list = [advance_id]


    def gen_output(self,case_type, case_id):
        if case_type == "basic":
            test_case = "%s/%s/%s.p" % (self.basic_case_dir, "test_cases", self.basic_cases[case_id])
            output_file = "%s/%s" % (self.output_dir, self.basic_cases[case_id])
        elif case_type == "advance":
            test_case = "%s/%s/%s.p" % (self.advance_case_dir, "test_cases", self.advance_cases[case_id])
            output_file = "%s/%s" % (self.output_dir, self.advance_cases[case_id])
        clist = [self.parser, test_case]
        try:
            proc = subprocess.Popen(clist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except Exception as e:
            print("Call of '%s' failed: %s" % (" ".join(clist), e))
            return False

        def convert_byte_seq_to_str(byte_seq):
            src = byte_seq.readlines()
            buffer = ""
            for line in src:
                try:
                    buffer += str(line, "utf-8")
                except UnicodeDecodeError as e:
                    print('\n'*3)
                    print(textwrap.indent(buffer, f'Last Output -> '))
                    print(f'  Non unicode output in output stream: {line}')
                    print(f'  Please remove non-unicode characters in your output')
                    print('\n'*3)
                    sys.exit(0)
            return buffer

        stdout = convert_byte_seq_to_str(proc.stdout)
        stderr = convert_byte_seq_to_str(proc.stderr)

        _ = proc.wait()
        with open(output_file, "w") as out:
            out.write(stdout)
            out.write(stderr)

    def test_sample_case(self, case_type, case_id):
        self.gen_output(case_type, case_id)

        if case_type == "basic":
            output_file = "%s/%s" % (self.output_dir, self.basic_cases[case_id])
            solution = "%s/%s/%s" % (self.basic_case_dir, "sample_solutions", self.basic_cases[case_id])
        elif case_type == "advance":
            output_file = "%s/%s" % (self.output_dir, self.advance_cases[case_id])
            solution = "%s/%s/%s" % (self.advance_case_dir, "sample_solutions", self.advance_cases[case_id])

        clist = ["diff", "-u", output_file, solution, f'--label="your output:({output_file})"', f'--label="answer:({solution})"']

        try:
            proc = subprocess.Popen(clist, stdout=subprocess.PIPE)
        except Exception as e:
            print("Call of '%s' failed: %s" % (" ".join(clist), e))
            return False


        output = str(proc.stdout.read(), "utf-8")
        retcode = proc.wait()
        if retcode != 0:
          if case_type == "basic":
            self.diff_result += "{}\n".format(self.basic_cases[case_id])
          elif case_type == "advance":
            self.diff_result += "{}\n".format(self.advance_cases[case_id])
          self.diff_result += "{}\n".format(output)

        return retcode == 0

    def run(self):
        print("---\tCase\t\tPoints")

        total_score = 0
        max_score = 0

        diff = open("{}/{}".format(self.output_dir, "diff.txt"), 'w')

        self.diff_result = ""
        for b_id in self.basic_id_list:
            c_name = self.basic_cases[b_id]
            print("+++ TESTING basic case %s:" % c_name)
            ok = self.test_sample_case("basic", b_id)
            max_val = self.basic_case_scores[b_id]
            get_val = max_val if ok else 0
            self.set_text_color(ok)
            print("---\t%s\t%d/%d" % (c_name, get_val, max_val))
            self.reset_text_color()
            total_score += get_val
            max_score += max_val

        for a_id in self.advance_id_list:
            c_name = self.advance_cases[a_id]
            print("+++ TESTING advance case %s:" % c_name)
            ok = self.test_sample_case("advance", a_id)
            max_val = self.advance_case_scores[a_id]
            get_val = max_val if ok else 0
            self.set_text_color(ok)
            print("---\t%s\t%d/%d" % (c_name, get_val, max_val))
            self.reset_text_color()
            total_score += get_val
            max_score += max_val

        self.set_text_color(total_score == max_score)
        print("---\tTOTAL\t\t%d/%d" % (total_score, max_score))
        self.reset_text_color()

        with open("{}/{}".format(self.output_dir, "score.txt"), "w") as result:
            result.write("---\tTOTAL\t\t%d/%d" % (total_score, max_score))

        diff.write(self.diff_result)
        diff.close()

    @staticmethod
    def set_text_color(test_passed: bool) -> None:
        """Sets the color based on whether the test has passed or not."""
        if test_passed:
            color = colorama.Fore.GREEN
        else:
            color = colorama.Fore.RED
        print(color, end='')

    @staticmethod
    def reset_text_color() -> None:
        print(colorama.Style.RESET_ALL, end='')


def main():
    parser = ArgumentParser()
    parser.add_argument("--parser", help="parser to test", default="../src/parser"
)
    parser.add_argument("--basic_case_id", help="basic case's ID", type=int, default=
0)
    parser.add_argument("--advance_case_id", help="advance case's ID", type=int, default=0)
    args = parser.parse_args()

    g = Grader(parser = args.parser)
    g.get_case_id_list(args.basic_case_id, args.advance_case_id)
    g.run()

if __name__ == "__main__":
    main()
