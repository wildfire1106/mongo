// expression.h

/**
 *    Copyright (C) 2013 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "mongo/base/disallow_copying.h"
#include "mongo/base/status.h"
#include "mongo/bson/bsonobj.h"
#include "mongo/db/matcher/matchable.h"
#include "mongo/db/matcher/match_details.h"

namespace mongo {

    class TreeMatchExpression;

    class MatchExpression {
        MONGO_DISALLOW_COPYING( MatchExpression );
    public:
        enum MatchType {
            // tree types
            AND, OR, NOR, NOT,

            // array types
            ALL, ELEM_MATCH_OBJECT, ELEM_MATCH_VALUE, SIZE,

            // leaf types
            LTE, LT, EQ, GT, GTE, REGEX, MOD, EXISTS, MATCH_IN, NIN,

            // special types
            TYPE_OPERATOR, GEO, WHERE,

            // things that maybe shouldn't even be nodes
            ATOMIC, ALWAYS_FALSE
        };

        MatchExpression( MatchType type );
        virtual ~MatchExpression(){}

        /**
         * determins if the doc matches the expression
         * there could be an expression that looks at fields, or the entire doc
         */
        virtual bool matches( const MatchableDocument* doc, MatchDetails* details = 0 ) const = 0;

        virtual bool matchesBSON( const BSONObj& doc, MatchDetails* details = 0 ) const;

        /**
         * does the element match the expression
         * not valid for all expressions ($where) where this will immediately return false
         */
        virtual bool matchesSingleElement( const BSONElement& e ) const = 0;

        virtual size_t numChildren() const { return 0; }
        virtual const MatchExpression* getChild( size_t i ) const { return NULL; }

        MatchType matchType() const { return _matchType; }

        virtual string toString() const;
        virtual void debugString( StringBuilder& debug, int level = 0 ) const = 0;

        virtual bool equivalent( const MatchExpression* other ) const = 0;
    protected:
        void _debugAddSpace( StringBuilder& debug, int level ) const;

    private:
        MatchType _matchType;
    };

    /**
     * this isn't really an expression, but a hint to other things
     * not sure where to put it in the end
     */
    class AtomicMatchExpression : public MatchExpression {
    public:
        AtomicMatchExpression() : MatchExpression( ATOMIC ){}

        virtual bool matches( const MatchableDocument* doc, MatchDetails* details = 0 ) const {
            return true;
        }

        virtual bool matchesSingleElement( const BSONElement& e ) const {
            return true;
        }

        virtual void debugString( StringBuilder& debug, int level = 0 ) const;

        virtual bool equivalent( const MatchExpression* other ) const {
            return other->matchType() == ATOMIC;
        }

    };

    class FalseMatchExpression : public MatchExpression {
    public:
        FalseMatchExpression() : MatchExpression( ALWAYS_FALSE ){}

        virtual bool matches( const MatchableDocument* doc, MatchDetails* details = 0 ) const {
            return false;
        }

        virtual bool matchesSingleElement( const BSONElement& e ) const {
            return false;
        }

        virtual void debugString( StringBuilder& debug, int level = 0 ) const;

        virtual bool equivalent( const MatchExpression* other ) const {
            return other->matchType() == ALWAYS_FALSE;
        }

    };
}
