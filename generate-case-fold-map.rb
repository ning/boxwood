#!/usr/bin/ruby

# Copyright 2010 Ning, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.


# The way as_utf8 works is suggested by the technique oulined here:
#    http://www.davidflanagan.com/2007/08/nifty-ruby-unicode-codepoints-utility.html
class String
  def as_utf8
    [self.to_i(16)].pack("U")
  end
end

calls = []

File.foreach('CaseFolding.txt') do |line|
  next if line[0] == '#'
  upper, status, lower, description = line.rstrip.split('; ')
  next unless ((status == 'C') || (status == 'S')) 
  upper = (upper.as_utf8.length > 1) ? "\\u#{upper}" : upper.as_utf8 
  lower = (lower.as_utf8.length > 1) ? "\\u#{lower}" : lower.as_utf8 
  calls << "case_fold_map_add(root, \"#{upper}\", \"#{lower}\");"
end


puts <<HEADER
/** Copyright 2010 Ning, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "case-fold.h"

void case_fold_map_load(struct case_fold_branch_t *root) {
  #{calls.join("\n  ")}
}
HEADER
